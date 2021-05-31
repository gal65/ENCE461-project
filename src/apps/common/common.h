#ifndef COMMON_H
#define COMMON_H

#include "pio.h"
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
void write_servo_bitbang(pio_t pio, uint8_t position);

#define SERVO_MIN_DUTY_MS 0.6
#define SERVO_MAX_DUTY_MS 2.7

void init_servo(pio_t pio);
void set_servo(uint8_t position);

#endif