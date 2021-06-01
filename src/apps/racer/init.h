#ifndef INIT_H
#define INIT_H

#include "adc.h"
#include "ledbuffer.h"
#include "nrf24.h"
#include "pwm.h"
#include "spi.h"

extern ledbuffer_t* leds;
extern pwm_t left_motor_pwm;
extern pwm_t pwm2;
extern pwm_t right_motor_pwm;
extern pwm_t pwm4;
extern nrf24_t* nrf;
extern spi_t spi;
extern adc_t battery_sensor;

#define NUM_LEDS 26

#define ENABLE_USB 0

void init_racer(void);
#endif