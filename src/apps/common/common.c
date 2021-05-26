#include "common.h"

#include <stdio.h>
// #include "nrf24.h"

// movement_data_t read_movement_data(nrf24_t nrf) {
//     static movement_data_t data;
//     nrf24_read(nrf, &data, sizeof(movement_data_t));
//     return data;
// }

void print_movement_data(movement_data_t data) {
    printf("left: %d %s, right %d %s\n",
        data.left_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD",
        data.right_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD"
    );
}