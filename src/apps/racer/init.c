#include "init.h"
#include "pwm.h"
#include "pio.h"
#include "target.h"
#include "pacer.h"
//radio
#include "nrf24.h"
#include "stdio.h"
#include <stdlib.h>
#include "delay.h"
#include "usb_serial.h"
#include "ledbuffer.h"
#include "common.h"

//battery test
#include "adc.h"

//LED strip
#define NUM_LEDS 26
#define LEDTAPE_PIO PB0_PIO

//AIN1
#define PWM1_PIO PA20_PIO
//AIN2
#define PWM2_PIO PA19_PIO
//BIN1
#define PWM3_PIO PA16_PIO
//BIN2
#define PWM4_PIO PA17_PIO
//nSLEEP
#define nSLP_PIO PA29_PIO

#define PWM_FREQ_HZ 100e3

//define bumper pin
#define BUMPER_DETECT PA30_PIO

//radio select
#define TOP_SW PB14_PIO
#define BOT_SW PB13_PIO

//battery 
#define BATTERY_VOLTAGE_ADC ADC_CHANNEL_8
/* Define how fast ticks occur.  This must be faster than
   TICK_RATE_MIN.  */
enum {LOOP_POLL_RATE = 20};

/* Define LED flash rate in Hz.  */
enum {LED_FLASH_RATE = 1};


static const pwm_cfg_t pwm1_cfg =
{
    .pio = PWM1_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t pwm2_cfg =
{
    .pio = PWM2_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t pwm3_cfg =
{
    .pio = PWM3_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

static const pwm_cfg_t pwm4_cfg =
{
    .pio = PWM4_PIO,
    .period = PWM_PERIOD_DIVISOR (PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR (PWM_FREQ_HZ, 50),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};




//paninc function for the radio
static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(1000);
    }
}

//battery detection set up
static adc_t battery_sensor;

static int battery_sensor_init(void)
{
    adc_cfg_t bat = {
        .channel = BATTERY_VOLTAGE_ADC,
        .bits = 12,
        .trigger = ADC_TRIGGER_SW,
        .clock_speed_kHz = F_CPU / 4000,
    };

    battery_sensor = adc_init(&bat);

    return (battery_sensor == 0) ? -1 : 0;
}

static uint16_t battery_millivolts(void)
{
    adc_sample_t s;
    adc_read(battery_sensor, &s, sizeof(s));

    // 5.6 pull down & 10k pull up gives a scale factor or
    // 5.6 / (5.6 + 10) = 0.3590
    // 4096 (max ADC reading) * 0.3590 ~= 1365
    return (uint16_t)((int)s) * 3300 / 1365;
}