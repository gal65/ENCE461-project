#ifndef INIT_H
#define INIT_H

#include "adc.h"
#include "mpu9250.h"
#include "nrf24.h"

extern nrf24_t* nrf;
extern adc_t joystick_x_adc;
extern adc_t joystick_y_adc;
extern adc_t battery_voltage_adc;
extern mpu_t* imu;

void init_hat(void);

#endif