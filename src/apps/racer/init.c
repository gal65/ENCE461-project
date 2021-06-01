#include "init.h"
#include "adc.h"
#include "common.h"
#include "delay.h"
#include "ledbuffer.h"
#include "nrf24.h"
#include "pio.h"
#include "pwm.h"
#include "target.h"
#include "usb_serial.h"
#include <stdio.h>

#define PWM_FREQ_HZ 100e3

//battery
#define BATTERY_VOLTAGE_ADC ADC_CHANNEL_8

static const pwm_cfg_t left_motor_pwm_cfg = {
    .pio = LEFT_MOTOR_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR(PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR(PWM_FREQ_HZ, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

// static const pwm_cfg_t pwm2_cfg = {
//     .pio = LEFT_MOTOR_DIRECTION_PIO,
//     .period = PWM_PERIOD_DIVISOR(PWM_FREQ_HZ),
//     .duty = PWM_DUTY_DIVISOR(PWM_FREQ_HZ, 0),
//     .align = PWM_ALIGN_LEFT,
//     .polarity = PWM_POLARITY_HIGH,
//     .stop_state = PIO_OUTPUT_LOW
// };

static const pwm_cfg_t right_motor_pwm_cfg = {
    .pio = RIGHT_MOTOR_PWM_PIO,
    .period = PWM_PERIOD_DIVISOR(PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR(PWM_FREQ_HZ, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_LOW,
    .stop_state = PIO_OUTPUT_LOW
};

// static const pwm_cfg_t pwm4_cfg = {
//     .pio = RIGHT_MOTOR_DIRECTION_PIO,
//     .period = PWM_PERIOD_DIVISOR(PWM_FREQ_HZ),
//     .duty = PWM_DUTY_DIVISOR(PWM_FREQ_HZ, 0),
//     .align = PWM_ALIGN_LEFT,
//     .polarity = PWM_POLARITY_HIGH,
//     .stop_state = PIO_OUTPUT_LOW
// };

#if ENABLE_USB
static usb_serial_cfg_t usb_serial_cfg = {
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};
#endif

//battery detection set up
adc_t battery_sensor;

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

//LED strip
ledbuffer_t* leds;

pwm_t left_motor_pwm;
// pwm_t pwm2;
pwm_t right_motor_pwm;
// pwm_t pwm4;

nrf24_t* nrf;
spi_t spi;

//paninc function for the radio
static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(1000);
    }
}

void init_racer(void)
{
    left_motor_pwm = pwm_init(&left_motor_pwm_cfg);
    // pwm2 = pwm_init(&pwm2_cfg);
    right_motor_pwm = pwm_init(&right_motor_pwm_cfg);
    // pwm4 = pwm_init(&pwm4_cfg);

    // init_servo();

    /* Configure LED PIO as output.  */
    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);

    pio_config_set(SERVO_PIO, PIO_OUTPUT_LOW);

    //configure Bumper to be NO(normally open) to set bumper_activite to high
    pio_config_set(BUMPER_DETECT, PIO_INPUT_PULLUP);

    //RADIO select changing address 11 -> 100, 10 -> 90, 01 -> 80, 00 -> 70
    pio_config_set(TOP_SW, PIO_INPUT_PULLUP);
    pio_config_set(BOT_SW, PIO_INPUT_PULLUP);

    pio_config_set(LEFT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);
    pio_config_set(RIGHT_MOTOR_DIRECTION_PIO, PIO_OUTPUT_LOW);

    //Start pwm channels
    // pwm_channels_start(pwm_channel_mask(servo_pwm) | pwm_channel_mask(left_motor_pwm) | pwm_channel_mask(pwm2) | pwm_channel_mask(right_motor_pwm) | pwm_channel_mask(pwm4));
    pwm_channels_start(pwm_channel_mask(servo_pwm) | pwm_channel_mask(left_motor_pwm) | pwm_channel_mask(right_motor_pwm));
    pio_config_set(nSLP_PIO, PIO_OUTPUT_HIGH);

    //radio part
    spi_cfg_t nrf_spi = {
        .channel = 0,
        .clock_speed_kHz = 1000,
        .cs = RADIO_CS_PIO,
        .mode = SPI_MODE_0,
        .cs_mode = SPI_CS_MODE_FRAME,
        .bits = 8,
    };

#if ENABLE_USB
    usb_serial_init(&usb_serial_cfg, "/dev/usb_tty");

    freopen("/dev/usb_tty", "a", stdout);
    freopen("/dev/usb_tty", "r", stdin);
#endif
    spi = spi_init(&nrf_spi);
    nrf = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf)
        panic();

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 15, 100, 32))
        panic();
    if (!nrf24_listen(nrf))
        panic();
    //
    if (battery_sensor_init() < 0)
        panic();
    leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);
}