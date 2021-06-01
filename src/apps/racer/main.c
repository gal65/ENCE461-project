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

uint8_t servo_pos;
#define PACER_RATE_HZ 50

#define BUMPER_TIMER_TICKS (PACER_RATE_HZ * 5)
#define BUMPER_HOLDOFF_TIMER_TICKS (PACER_RATE_HZ * 1)

// half second motor timeout
#define MOTOR_TIMEOUT (PACER_RATE_HZ / 2)

typedef enum {
    BUMPER_WAITING,
    BUMPER_HOLDOFF,
    NORMAL
} state_t;

uint16_t key = 0;

state_t state = NORMAL;
int waiting_counter = 0;
int holdoff_counter = 0;

int color_update_counter = 0;

int motor_timeout_counter = 0;

int main(void)
{
    // init_servo();
    // for (;;) {
    //     set_servo(0);
    //     delay_ms(500);
    //     set_servo(255);
    //     delay_ms(500);
    // }
    init_racer();
    pacer_init(PACER_RATE_HZ);

    while (1) {
        pacer_wait();

        // servo timing
        pio_output_high(SERVO_PIO);
        if (servo_pos == 255) {
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
            pwm_duty_set(left_motor_pwm, 0);
            pio_config_set(LEFT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(right_motor_pwm, 0);
            pio_config_set(RIGHT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);
            char buffer1[32];
            sprintf(buffer1, "1\r\n");
            nrf24_write(nrf, buffer1, sizeof(buffer1));
            nrf24_listen(nrf);
            state = BUMPER_WAITING;
        }

        motor_timeout_counter++;

        mosi_comms_t rx;
        if (nrf24_read(nrf, &rx, sizeof(mosi_comms_t))) {
            if (rx.set_key) {
                motor_timeout_counter = 0;
                key = rx.key;
            } else if (rx.key == key) {
                // only accept packet if it contains the same key that was sent

                motor_timeout_counter = 0;
#if ENABLE_USB
                print_mosi_comms(rx);
                fflush(stdout);
#endif
                pio_output_toggle(LED2_PIO);

                if (state != BUMPER_WAITING) {
                    pwm_duty_set(left_motor_pwm, rx.left_motor_pwm);
                    pwm_duty_set(right_motor_pwm, rx.right_motor_pwm);
                    pio_output_set(LEFT_MOTOR_DIRECTION_PIO, rx.left_motor_direction);
                    pio_output_set(RIGHT_MOTOR_DIRECTION_PIO, rx.right_motor_direction);
                }
                servo_pos = rx.servo_position;
            }
        }

        if (motor_timeout_counter >= MOTOR_TIMEOUT) {
            pwm_duty_set(left_motor_pwm, 0);
            pio_config_set(LEFT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(right_motor_pwm, 0);
            pio_config_set(RIGHT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);
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
    color_update_counter++;
    if (color_update_counter % 1 == 0) {
        if (state == NORMAL) {
            color += 8;
            ledbuffer_set(leds, 0, 0, color < 128 ? 128 - color : color - 128, color < 128 ? color : 128 - (color - 128));
        } else if (state == BUMPER_WAITING) {
            ledbuffer_set(leds, 0, 100, 0, 0);
        } else if (state == BUMPER_HOLDOFF) {
            ledbuffer_set(leds, 0, 0, 0, color_update_counter % 4 == 0 ? 128 : 0);
        }

        ledbuffer_write(leds);
        ledbuffer_advance(leds, 1);
    }
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