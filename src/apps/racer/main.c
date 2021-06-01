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

/* Define how fast ticks occur.  This must be faster than
   TICK_RATE_MIN.  */
enum { LOOP_POLL_RATE = 20 };

/* Define LED flash rate in Hz.  */
enum { LED_FLASH_RATE = 1 };

static uint16_t battery_millivolts(void);

int main(void)
{
    //LED strip
    bool blue = false;
    int count = 0;

    init_racer();

    uint8_t flash_ticks;

    pacer_init(LOOP_POLL_RATE);
    flash_ticks = 0;

    while (1) {
        pacer_wait();

        pio_output_toggle(LED1_PIO);
        if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 1) {
            nrf24_set_address(nrf, 100);
        } else if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 0) {
            nrf24_set_address(nrf, 90);
        } else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 1) {
            nrf24_set_address(nrf, 80);
        } else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 0) {
            nrf24_set_address(nrf, 70);
        }
        if (pio_input_get(BUMPER_DETECT) == 0) {
            pwm_duty_set(pwm1, 0);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(pwm3, 0);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW); //stop
            char buffer1[32];
            sprintf(buffer1, "1\r\n");
            nrf24_write(nrf, buffer1, sizeof(buffer1));
            delay_ms(5000);
            nrf24_listen(nrf);
        }

        if (battery_millivolts() < 6400) {
            pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);
        } else {
            pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);
        }
        printf("%d\n", battery_millivolts());
        //printf("%d\n", pio_input_get(BOT_SW));
        /* Wait until next clock tick.  */

        //LED strip
        if (count++ == NUM_LEDS) {
            // wait for a revolution
            ledbuffer_clear(leds);
            if (blue) {
                ledbuffer_set(leds, 0, 0, 0, 100);
                ledbuffer_set(leds, NUM_LEDS / 2, 0, 0, 100);
            } else {
                ledbuffer_set(leds, 0, 100, 0, 0);
                ledbuffer_set(leds, NUM_LEDS / 2, 100, 0, 0);
            }
            blue = !blue;
            count = 0;
        }

        ledbuffer_write(leds);
        ledbuffer_advance(leds, 1);
        //LED strip

        char buffer[32];
        if (nrf24_read(nrf, buffer, sizeof(buffer))) {
#if ENABLE_USB
            printf("%s\n", buffer);
            printf("%d\n", atoi(&buffer[12]));
#endif
            //pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED2_PIO);
            fflush(stdout);

            set_servo(atoi(&buffer[14]));

            int duty_1 = atoi(&buffer[4]);
            pwm_duty_set(pwm1, duty_1);
            int dir_1 = atoi(&buffer[0]);
            if (dir_1 == 0) {
                pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            } else {
                pio_config_set(PWM2_PIO, PIO_OUTPUT_HIGH);
            }
            int duty_2 = atoi(&buffer[9]);
            pwm_duty_set(pwm3, duty_2);
            int dir_2 = atoi(&buffer[2]);
            if (dir_2 == 0) {
                pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);
            } else {
                pio_config_set(PWM4_PIO, PIO_OUTPUT_HIGH);
            }
        }
        //change servo position
        // int servo = atoi(&buffer[15]);
        // set_servo(servo);
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