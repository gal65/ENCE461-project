#include "common.h"
#include "delay.h"
#include "pio.h"

#include <stdio.h>
// #include "nrf24.h"

// motor_data_t read_movement_data(nrf24_t nrf) {
//     static motor_data_t data;
//     nrf24_read(nrf, &data, sizeof(motor_data_t));
//     return data;
// }

void print_movement_data(motor_data_t data)
{
    printf("left: %lu %s, right %lu %s\n",
        data.left_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD",
        data.right_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD");
}

// positino 0 -> 255
#define SERVO_MIN_US 600
#define SERVO_MAX_US 2800
#define SERVO_PWM_PERIOD_US 20000

void write_servo(pio_t pio, uint8_t position)
{
    int us = position * 2200 / 255 + 600;
    int rest = 20000 - us;
    for (int i = 0; i < 3; i++) {
        pio_output_high(pio);
        DELAY_US(us);
        pio_output_low(pio);
        DELAY_US(rest);
    }
}