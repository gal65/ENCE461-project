#include <stdlib.h>

#include "adc.h"
#include "common.h"
#include "config.h"
#include "control_mapping.h"
#include "init.h"
#include "kernel.h"
#include "mcu_sleep.h"
#include "mpu9250.h"
#include "nrf24.h"
#include "pio.h"
#include "stdio.h"
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

bool control_from_imu = true;

void imu_task(void)
{
    int16_t accel[3];
    char radio_send_buffer[32] = { 0 };

    if (!control_from_imu) {
        return;
    }

    if (!imu) {
        printf("ERROR: can't find MPU9250!\n");
        fflush(stdout);
        return;
    }

    if (!mpu9250_is_imu_ready(imu)) {
        printf("Waiting for IMU to be ready...\n");
        fflush(stdout);
        return;
    }

    if (!mpu9250_read_accel(imu, accel)) {
        printf("ERROR: failed to read acceleration\n");
        fflush(stdout);
        return;
    }

    motor_data_t move = get_motor_values_imu(accel);

    snprintf(radio_send_buffer, sizeof(radio_send_buffer), "%d %d %-4lu %-4lu",
        move.left_motor_direction, move.right_motor_direction,
        move.left_motor_pwm, move.right_motor_pwm);

    printf("%s\n", radio_send_buffer);
    fflush(stdout);

    nrf24_write(nrf, radio_send_buffer, sizeof(radio_send_buffer));

    pio_output_toggle(LED1_PIO);
}

void joystick_task(void)
{
    char radio_send_buffer[32] = { 0 };
    if (control_from_imu) {
        return;
    }

    uint16_t x_data, y_data;
    adc_read(adc_1, &x_data, sizeof(x_data));
    adc_read(adc_2, &y_data, sizeof(y_data));

    motor_data_t move = get_motor_values_joystick(x_data, y_data);

    snprintf(radio_send_buffer, sizeof(radio_send_buffer), "%d %d %-4lu %-4lu",
        move.left_motor_direction, move.right_motor_direction,
        move.left_motor_pwm, move.right_motor_pwm);

    printf("%s\n", radio_send_buffer);
    fflush(stdout);
}

// Allows switching between imu_task and joystick_task depending on
// the value of `control_from_imu`
void control_method_button(void)
{
    static bool prev_button_state = true;
    bool button_state = pio_input_get(BUTTON_PIO);

    if (prev_button_state && !button_state) {
        control_from_imu = !control_from_imu;
        printf("Control from IMU: %d\n", control_from_imu);
        fflush(stdout);
    }
    prev_button_state = button_state;
}

int main(void)
{
    init_hat();

    task_t tasks[] = {
        { imu_task, 100, 0 },
        { joystick_task, 100, 0 },
        { control_method_button, 100, 0 },
    };

    kernel_run(tasks, sizeof(tasks) / sizeof(task_t));

    pacer_init(10);
    // tweeter_init(&tweety, 1000, &scale_table);

    while (1) {
        pacer_wait();
        // tweeter_update(tweety);
        // sprintf(adc_1, "\n");
        // sprintf(adc_2, "\n");
        // sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
        // nrf24_write(nrf, buffer, sizeof(buffer));
        // if (pio_input_get(BUTTON_PIO))
        //{
        //    delay_ms(1000);
        //    mcu_sleep_wakeup_set(&wake_up_pin);
        //    mcu_sleep(&sleepy_mode);
        //}
    }
}