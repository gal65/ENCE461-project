/* File:   pwm_test2.c
   Author: M. P. Hayes, UCECE
   Date:   15 April 2013
   Descr:  This example starts two channels simultaneously; one inverted
           with respect to the other.
*/
#include "pacer.h"
#include "pio.h"
#include "pwm.h"
#include "target.h"
//radio
#include "common.h"
#include "delay.h"
#include "init.h"
#include "ledbuffer.h"
#include "nrf24.h"
#include "stdio.h"
#include "usb_serial.h"
#include <stdlib.h>

//battery test
#include "adc.h"

static uint16_t battery_millivolts(void);
static void update_leds(void);
static void update_radio_channel(void);

uint8_t current_servo_pos;
#define PACER_RATE_HZ 50

#define BUMPER_TIMER_TICKS (PACER_RATE_HZ * 5)
#define BUMPER_HOLDOFF_TIMER_TICKS (PACER_RATE_HZ * 2)

typedef enum {
    BUMPER_WAITING,
    BUMPER_HOLDOFF,
    NORMAL
} state_t;

state_t state = NORMAL;
int waiting_counter = 0;
int holdoff_counter = 0;

int main(void)
{
    init_racer();
    pacer_init(PACER_RATE_HZ);

    while (1) {
        pacer_wait();

        // servo timing
        pio_output_high(SERVO_PIO);
        if (current_servo_pos == 255) {
            DELAY_US(SERVO_MAX_DUTY_MS * 1000);
        } else {
            DELAY_US(SERVO_MIN_DUTY_MS * 1000);
        }
        pio_output_low(SERVO_PIO);

        pio_output_toggle(LED1_PIO);

        if (battery_millivolts() < 6400) {
            pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);
        } else {
            pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);
        }

        update_radio_channel();
        update_leds();

        if (state == BUMPER_WAITING) {
            waiting_counter++;
            if (waiting_counter >= BUMPER_TIMER_TICKS) {
                waiting_counter = 0;
                state = BUMPER_HOLDOFF;
            }
        } else if (state == BUMPER_HOLDOFF) {
            holdoff_counter++;
            if (holdoff_counter >= BUMPER_HOLDOFF_TIMER_TICKS) {
                holdoff_counter = 0;
                state = NORMAL;
            }
        }

        if ((state != BUMPER_HOLDOFF) && (!pio_input_get(BUMPER_DETECT))) {
            pwm_duty_set(pwm1, 0);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(pwm3, 0);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW); //stop
            char buffer1[32];
            sprintf(buffer1, "1\r\n");
            nrf24_write(nrf, buffer1, sizeof(buffer1));
            nrf24_listen(nrf);
            state = BUMPER_WAITING;
        }

        //printf("%d\n", pio_input_get(BOT_SW));
        /* Wait until next clock tick.  */

        //LED strip

        char buffer[32];
        mosi_comms_t rx;
        if (nrf24_read(nrf, &rx, sizeof(mosi_comms_t))) {
#if ENABLE_USB
            // printf("battery %d\n", battery_millivolts());
            // printf("rx %s\n", buffer);
            // printf("rx[12] %d\n", atoi(&buffer[12]));
            print_mosi_comms(rx);
            fflush(stdout);
#endif
            //pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED2_PIO);

#if 1
            pwm_duty_set(pwm1, rx.left_motor_pwm);
            pwm_duty_set(pwm3, rx.right_motor_pwm);
            pio_output_set(PWM2_PIO, rx.left_motor_direction);
            pio_output_set(PWM4_PIO, rx.right_motor_direction);
            // int duty_1 = atoi(&buffer[4]);
            // pwm_duty_set(pwm1, duty_1);

            // int dir_1 = atoi(&buffer[0]);
            // if (dir_1 == 0) {
            //     pio_output_low(PWM2_PIO);
            // } else {
            //     pio_output_high(PWM2_PIO);
            // }

            // int duty_2 = atoi(&buffer[9]);
            // pwm_duty_set(pwm3, duty_2);

            // int dir_2 = atoi(&buffer[2]);
            // if (dir_2 == 0) {
            //     pio_output_low(PWM4_PIO);
            // } else {
            //     pio_output_high(PWM4_PIO);
            // }
#endif

            current_servo_pos
                = atoi(&buffer[14]);
        }
    }

    return 0;
}

static uint16_t battery_millivolts(void)
{
    adc_sample_t s;
    adc_read(battery_sensor, &s, sizeof(s));

    // 5.6 pull down & 10k pull up gives a scale factor or
    // 5.6 / (5.6 + 10) = 0.3590
    // 4096 (max ADC reading) * 0.3590 ~= 1365
    return (uint16_t)((int)s) * 3300 / 1365;
}

uint8_t color = 0;
static void update_leds(void)
{
    if (state == NORMAL) {
        color += 8;
        ledbuffer_set(leds, 0, 0, color < 128 ? 128 - color : color - 128, color < 128 ? color : 128 - (color - 128));
    } else if (state == BUMPER_WAITING) {
        ledbuffer_set(leds, 0, 100, 0, 0);
    } else if (state == BUMPER_HOLDOFF) {
        ledbuffer_set(leds, 0, 0, 0, holdoff_counter % 4 == 0 ? 128 : 0);
    }

    ledbuffer_write(leds);
    ledbuffer_advance(leds, 1);
}

static void update_radio_channel(void)
{
    if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 1) {
        nrf24_set_address(nrf, 100);
    } else if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 0) {
        nrf24_set_address(nrf, 90);
    } else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 1) {
        nrf24_set_address(nrf, 80);
    } else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 0) {
        nrf24_set_address(nrf, 70);
    }
}