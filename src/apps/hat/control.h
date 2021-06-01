#ifndef CONTROL_H
#define CONTROL_H

#include "common.h"
#include <stdint.h>

int32_t apply_response_curve(int32_t input, int32_t zero_thresh, int32_t sat_thresh, int32_t sat_output);
mosi_comms_t get_motor_values_imu(int16_t* accel_data);
mosi_comms_t get_motor_values_joystick(uint16_t x_data, uint16_t y_data);

void imu_control_task(void);
void joystick_control_task(void);
void change_control_method_task(void);
uint8_t update_servo_position_task(void);
void generate_key_task(void);

#endif