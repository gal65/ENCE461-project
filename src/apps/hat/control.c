#include "control.h"
#include "init.h"
#include "kernel.h"
#include "sound.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FIXED_POINT_EXP 1000

int32_t apply_response_curve(int32_t input, int32_t zero_thresh, int32_t sat_thresh, int32_t sat_output)
{
    int32_t input_abs = abs(input);
    int32_t linear_region_divisor_ks = (sat_thresh - zero_thresh) * FIXED_POINT_EXP / sat_output;
    if (input_abs < zero_thresh) {
        return 0;
    } else if (input > sat_thresh) {
        return sat_output;
    } else if (input < -sat_thresh) {
        return -sat_output;
    } else {
        input_abs = input_abs - zero_thresh;
        input_abs *= FIXED_POINT_EXP;
        input_abs /= linear_region_divisor_ks;
        return input > 0 ? input_abs : -input_abs;
    }
}

motor_data_t pwm_from_xy(int32_t forward_back, int32_t left_right)
{
    motor_data_t move;

    move.left_motor_direction = forward_back > 0 ? FORWARD : BACKWARD;
    move.left_motor_pwm = abs(forward_back);

    move.right_motor_direction = move.left_motor_direction;
    move.right_motor_pwm = move.left_motor_pwm;

    if (left_right > 0) {
        // turning to the right
        move.right_motor_pwm *= (1000 - abs(left_right));
        move.right_motor_pwm /= 1000;
    } else if (left_right < 0) {
        // turning to the left
        move.left_motor_pwm *= (1000 - abs(left_right));
        move.left_motor_pwm /= 1000;
    }

    if (move.left_motor_direction == BACKWARD) {
        move.left_motor_pwm = 1000 - move.left_motor_pwm;
    }
    if (move.right_motor_direction == BACKWARD) {
        move.right_motor_pwm = 1000 - move.right_motor_pwm;
    }
    return move;
}

motor_data_t get_motor_values_imu(int16_t* accel_data)
{
    int32_t forward_back = apply_response_curve(accel_data[1], 3000, 6000, 1000);
    int32_t left_right = apply_response_curve(accel_data[0], 4000, 6000, 1000);
    return pwm_from_xy(forward_back, left_right);
}

#define X_LEFT 3800
#define X_RIGHT 250
#define Y_UP 60
#define Y_DOWN 3800

motor_data_t get_motor_values_joystick(uint16_t x_data, uint16_t y_data)
{
    motor_data_t move;
    int32_t conditioned_x = ((int32_t)x_data - X_RIGHT) * 2000 / (X_LEFT - X_RIGHT) - 1000;
    int32_t conditioned_y = ((int32_t)y_data - Y_UP) * 2000 / (Y_DOWN - Y_UP) - 1000;

    // invert both response curves due to joystick orientation
    conditioned_x = -apply_response_curve(conditioned_x, 100, 900, 1000);
    conditioned_y = -apply_response_curve(conditioned_y, 100, 400, 1000);

    return pwm_from_xy(conditioned_y, conditioned_x);
}

// TASKS:

bool control_from_imu = true;

void imu_control_task(void)
{
    int16_t accel[3];
    char radio_send_buffer[32] = { 0 };

    if (!imu) {
#if USB_DEBUG
        printf("ERROR: can't find MPU9250!\n");
        fflush(stdout);
#endif
        return;
    }

    if (!mpu9250_is_imu_ready(imu)) {
#if USB_DEBUG
        printf("Waiting for IMU to be ready...\n");
        fflush(stdout);
#endif
        return;
    }

    if (!mpu9250_read_accel(imu, accel)) {
#if USB_DEBUG
        printf("ERROR: failed to read acceleration\n");
        fflush(stdout);
#endif
        return;
    }

    motor_data_t move = get_motor_values_imu(accel);

    snprintf(radio_send_buffer, sizeof(radio_send_buffer), "%d %d %-4lu %-4lu",
        move.left_motor_direction, move.right_motor_direction,
        move.left_motor_pwm, move.right_motor_pwm);

#if USB_DEBUG
    printf("%s\n", radio_send_buffer);
    fflush(stdout);
#endif

    nrf24_write(nrf, radio_send_buffer, sizeof(radio_send_buffer));
    nrf24_listen(nrf);
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

#if USB_DEBUG
    printf("%s\n", radio_send_buffer);
    fflush(stdout);
#endif

    nrf24_write(nrf, radio_send_buffer, sizeof(radio_send_buffer));
    nrf24_listen(nrf);
}

// Allows switching between imu_control_task and joystick_control_task depending on
// the value of `control_from_imu`
void change_control_method_task(void)
{
    static bool prev_button_state = true;
    bool button_state = pio_input_get(JOYSTICK_BUTTON_PIO);

    if (prev_button_state && !button_state) {
        control_from_imu = !control_from_imu;
        tweet_sound_play();

        if (control_from_imu) {
            disable_task("joystick_control");
            enable_task("imu_control");
        } else {
            enable_task("joystick_control");
            disable_task("imu_control");
        }

#if USB_DEBUG
        printf("Control from IMU: %d\n", control_from_imu);
        fflush(stdout);
#endif
    }
    prev_button_state = button_state;
}