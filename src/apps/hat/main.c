#include <stdlib.h>

#include "adc.h"
#include "common.h"
#include "config.h"
#include "control_mapping.h"
#include "delay.h"
#include "init.h"
#include "kernel.h"
#include "mcu_sleep.h"
#include "mpu9250.h"
#include "nrf24.h"
#include "pacer.h"
#include "pio.h"
#include "stdio.h"
#include "target.h"
#include "tweeter.h"

// static tweeter_scale_t scale_table[] = TWEETER_SCALE_TABLE (1000);

// static const tweeter_private_t tweety =
//{
//  .note_clock = 1000,
//  .note_period = 200,
//  .note_duty = 200,
//  .note_holdoff = 50e-3,
//  .poll_rate = 1000,
//  .scale_table = &scale_table,
//};

// static const mcu_sleep_wakeup_cfg_t wake_up_pin=
//{
//    .pio = PA2_PIO,
//    .active_high = false
//};

// static const mcu_sleep_cfg_t sleepy_mode =
//{
//    .mode = MCU_SLEEP_MODE_BACKUP
//};

int main(void)
{
    init_hat();

    pacer_init(10);
    // tweeter_init(&tweety, 1000, &scale_table);

    uint16_t data;
    while (1) {
        pacer_wait();
        // tweeter_update(tweety);
        adc_read(adc_1, &data, sizeof(data));
        adc_read(adc_2, &data, sizeof(data));
        sprintf(adc_1, "\n");
        sprintf(adc_2, "\n");
        // sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
        // nrf24_write(nrf, buffer, sizeof(buffer));
        // if (pio_input_get(BUTTON_PIO))
        //{
        //    delay_ms(1000);
        //    mcu_sleep_wakeup_set(&wake_up_pin);
        //    mcu_sleep(&sleepy_mode);
        //}
        if (imu) {
            /* read in the accelerometer data */
            if (!mpu9250_is_imu_ready(imu)) {
                printf("Waiting for IMU to be ready...\n");
            } else {
                int16_t accel[3];
                if (mpu9250_read_accel(imu, accel)) {
                    motor_data_t move = get_motor_values_imu(accel);
                    // nrf24_write(nrf, &move, sizeof(move));
                    char buffer[32] = { 0 };
                    snprintf(buffer, sizeof(buffer), "%d %d %-4lu %-4lu",
                        move.left_motor_direction, move.right_motor_direction,
                        move.left_motor_pwm, move.right_motor_pwm);
                    printf("%s\n", buffer);
                    nrf24_write(nrf, buffer, sizeof(buffer));
                    pio_output_toggle(LED1_PIO);
                } else {
                    printf("ERROR: failed to read acceleration\n");
                }
            }
        } else {
            printf("ERROR: can't find MPU9250!\n");
        }

        fflush(stdout);
    }
}
