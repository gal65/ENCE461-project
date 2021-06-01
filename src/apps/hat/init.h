#ifndef INIT_H
#define INIT_H

#include "adc.h"
#include "ledbuffer.h"
#include "mpu9250.h"
#include "nrf24.h"
#include "tweeter.h"

#define USB_DEBUG 0
#define LOW_POWER_FEATURE 0

extern nrf24_t* nrf;
extern adc_t joystick_x_adc;
extern adc_t joystick_y_adc;
extern adc_t battery_voltage_adc;
extern mpu_t* imu;
extern ledbuffer_t* led_buffer;
extern twi_t imu_twi;
extern spi_t nrf_spi;
extern tweeter_t tweeter;

void init_hat(void);

#endif