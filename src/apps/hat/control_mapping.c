#include "control_mapping.h"
#include <stdlib.h>
#include <stdint.h>

#define FIXED_POINT_EXP 1000

int32_t apply_response_curve(int32_t input, int32_t zero_thresh, int32_t sat_thresh, int32_t sat_output) {
    int32_t input_abs = abs(input);
    int32_t linear_region_divisor_ks = (sat_thresh - zero_thresh)*FIXED_POINT_EXP / sat_output;
    if(input_abs < zero_thresh) {
        return 0;
    } else if(input > sat_thresh) {
        return sat_output;
    } else if(input < -sat_thresh) {
        return -sat_output;
    } else {
        input_abs = input_abs - zero_thresh;
        input_abs *= FIXED_POINT_EXP;
        input_abs /= linear_region_divisor_ks;
        return input > 0 ? input_abs : -input_abs;
    }
}

motor_data_t get_motor_values_imu(int16_t* accel_data) {
    motor_data_t move;
    int32_t forward_back = apply_response_curve(accel_data[1], 3000, 6000, 1000);
    int32_t left_right = apply_response_curve(accel_data[0], 4000, 6000, 1000);

    move.left_motor_direction = forward_back > 0 ? FORWARD : BACKWARD;
    move.left_motor_pwm = abs(forward_back);

    move.right_motor_direction = move.left_motor_direction;
    move.right_motor_pwm = move.left_motor_pwm;

    if(left_right > 0) {
        // turning to the right
        move.right_motor_pwm *= (1000 - abs(left_right));
        move.right_motor_pwm /= 1000;
    } else if(left_right < 0){
        // turning to the left
        move.left_motor_pwm *= (1000 - abs(left_right));
        move.left_motor_pwm /= 1000;
    }

    if(move.left_motor_direction == BACKWARD) {
        move.left_motor_pwm = 1000 - move.left_motor_pwm;
    }
    if(move.right_motor_direction == BACKWARD) {
        move.right_motor_pwm = 1000 - move.right_motor_pwm;
    }
    return move;
}