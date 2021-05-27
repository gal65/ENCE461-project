#ifndef INIT_H
#define INIT_H

#include "adc.h"
#include "mpu9250.h"
#include "nrf24.h"

extern nrf24_t* nrf;
extern adc_t adc_1;
extern adc_t adc_2;
extern mpu_t* imu;

void init_hat(void);

#endif