#ifndef CONTROL_MAPPING_H
#define CONTROL_MAPPING_H

#include "common.h"
#include <stdint.h>

int32_t apply_response_curve(int32_t input, int32_t zero_thresh, int32_t sat_thresh, int32_t sat_output);
motor_data_t get_motor_values_imu(int16_t* accel_data);
motor_data_t get_motor_values_joystick(uint16_t x_data, uint16_t y_data);

#endif