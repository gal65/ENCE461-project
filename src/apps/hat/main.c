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
#include "pio.h"
#include "sleep.h"
#include "sound.h"
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

#define LOW_POWER_FEATURE 0
#define PRINT_CONTROLLER_OUTPUT 1

bool control_from_imu = true;

void imu_control_task(void)
{
    int16_t accel[3];
    char radio_send_buffer[32] = { 0 };

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

#if PRINT_CONTROLLER_OUTPUT
    printf("%s\n", radio_send_buffer);
    fflush(stdout);
#endif

    nrf24_write(nrf, radio_send_buffer, sizeof(radio_send_buffer));
    nrf24_listen(nrf);

    pio_output_toggle(LED1_PIO);
}

void joystick_control_task(void)
{
    char radio_send_buffer[32] = { 0 };

    uint16_t x_data, y_data;
    adc_read(joystick_x_adc, &x_data, sizeof(x_data));
    adc_read(joystick_y_adc, &y_data, sizeof(y_data));

    motor_data_t move = get_motor_values_joystick(x_data, y_data);

    snprintf(radio_send_buffer, sizeof(radio_send_buffer), "%d %d %-4lu %-4lu",
        move.left_motor_direction, move.right_motor_direction,
        move.left_motor_pwm, move.right_motor_pwm);

#if PRINT_CONTROLLER_OUTPUT
    printf("%s\n", radio_send_buffer);
    fflush(stdout);
#endif

    nrf24_write(nrf, radio_send_buffer, sizeof(radio_send_buffer));
    nrf24_listen(nrf);

    pio_output_toggle(LED1_PIO);
}

// Allows switching between imu_control_task and joystick_control_task depending on
// the value of `control_from_imu`
void change_control_method_task(void)
{
    static bool prev_button_state = true;
    bool button_state = pio_input_get(JOYSTICK_BUTTON_PIO);

    if (prev_button_state && !button_state) {
        control_from_imu = !control_from_imu;

        if (control_from_imu) {
            disable_task("joystick_control");
            enable_task("imu_control");
        } else {
            enable_task("joystick_control");
            disable_task("imu_control");
        }

        printf("Control from IMU: %d\n", control_from_imu);
        fflush(stdout);
    }
    prev_button_state = button_state;
}

void check_bumber_task(void)
{
    char buffer[32];
    if (nrf24_is_data_ready(nrf)) {
        nrf24_read(nrf, buffer, sizeof(buffer));
        // sound_play();
        printf("BUMPER\n");
        fflush(stdout);
    }
}

void battery_voltage_task(void)
{
#if LOW_POWER_FEATURE
    uint16_t bat;
    adc_read(battery_voltage_adc, &bat, sizeof(bat));

    if (bat < 2715) {
        pio_output_low(LED2_PIO);
        enter_low_power();
    } else {
        pio_output_high(LED2_PIO);
        exit_low_power();
    }
#endif
}

mcu_sleep_cfg_t sleep_cfg = {
    .mode = MCU_SLEEP_MODE_WAIT
};

mcu_sleep_wakeup_cfg_t wakeup_cfg = {
    .pio = BUTTON_PIO,
    .active_high = false
};

void check_sleep_mode_task(void)
{
    static bool prev_button_state = true;

    bool button_state = pio_input_get(BUTTON_PIO);

    if (prev_button_state && !button_state) {
        delay_ms(1000);
        mcu_sleep_wakeup_set(&wakeup_cfg);
        mcu_sleep(&sleep_cfg);
    }
    prev_button_state = button_state;
}

#define DEC_AMOUNT 20
uint8_t r_count = 255;
uint8_t b_count = 0;
uint8_t g_count = 0;
typedef enum {
    RED,
    GREEN,
    BLUE
} dec_color_t;

dec_color_t dec_color = RED;

void ledtape_update(void)
{
    ledbuffer_advance(led_buffer, 1);
    switch (dec_color) {
    case RED:
        r_count -= DEC_AMOUNT;
        g_count += DEC_AMOUNT;
        if (r_count <= DEC_AMOUNT) {
            dec_color = GREEN;
        }
        break;
    case GREEN:
        g_count -= DEC_AMOUNT;
        b_count += DEC_AMOUNT;
        if (g_count <= DEC_AMOUNT) {
            dec_color = BLUE;
        }
        break;
    case BLUE:
        b_count -= DEC_AMOUNT;
        r_count += DEC_AMOUNT;
        if (b_count <= DEC_AMOUNT) {
            dec_color = RED;
        }
        break;
    }
    ledbuffer_set(led_buffer, 0, r_count, g_count, b_count);
    ledbuffer_write(led_buffer);
}

void enter_low_power(void)
{
    ledbuffer_clear(led_buffer);
    ledbuffer_write(led_buffer);
    disable_task("leds");
}

void exit_low_power(void)
{
    enable_task("leds");
}

int main(void)
{
    init_hat();

    task_t tasks[] = {
        create_task("leds", ledtape_update, 50),
        create_task("imu_control", imu_control_task, 100),
        create_task("joystick_control", joystick_control_task, 100),
        create_task("change_control", change_control_method_task, 100),
        create_task("bumper", check_bumber_task, 100),
        create_task("sleep", check_sleep_mode_task, 100),

        // todo lower period
        create_task("battery", battery_voltage_task, 5000),
    };
    kernel_init(tasks, sizeof(tasks) / sizeof(task_t));

    // disable_task("joystick_control");
    kernel_run();

    // pacer_init(10);
    // tweeter_init(&tweety, 1000, &scale_table);

    // while (1) {
    // pacer_wait();
    // tweeter_update(tweety);
    // sprintf(joystick_x_adc, "\n");
    // sprintf(joystick_y_adc, "\n");
    // sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
    // nrf24_write(nrf, buffer, sizeof(buffer));
    // if (pio_input_get(BUTTON_PIO))
    //{
    //    delay_ms(1000);
    //    mcu_sleep_wakeup_set(&wake_up_pin);
    //    mcu_sleep(&sleepy_mode);
    //}
    // }
}