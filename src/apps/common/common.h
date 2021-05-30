#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
// #include "nrf24.h"

typedef enum {
    FORWARD = 0,
    BACKWARD = 1
} direction_t;

typedef struct {
    direction_t left_motor_direction;
    direction_t right_motor_direction;
    uint32_t left_motor_pwm;
    uint32_t right_motor_pwm;
} motor_data_t;

// motor_data_t read_movement_data(nrf24_t nrf);

void print_movement_data(motor_data_t data);

void imu_control_task(void);
void joystick_control_task(void);
void change_control_method_task(void);

#endif