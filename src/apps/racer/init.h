#ifndef INIT_H
#define INIT_H

#include "adc.h"
#include "ledbuffer.h"
#include "nrf24.h"
#include "pwm.h"
#include "spi.h"

extern ledbuffer_t* leds;
extern pwm_t pwm1;
extern pwm_t pwm2;
extern pwm_t pwm3;
extern pwm_t pwm4;
extern nrf24_t* nrf;
extern spi_t spi;
extern adc_t battery_sensor;

#define NUM_LEDS 26

void init_racer(void);
#endif