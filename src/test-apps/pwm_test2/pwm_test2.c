/* File:   pwm_test2.c
   Author: M. P. Hayes, UCECE
   Date:   15 April 2013
   Descr:  This example starts two channels simultaneously; one inverted
           with respect to the other.
*/
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


int
main (void)
{
    //LED strip
    bool blue = false;
    int count = 0;

    ledbuffer_t* leds = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);
    //LED strip
    pwm_t pwm1;
    pwm_t pwm2;
    pwm_t pwm3;
    pwm_t pwm4;

    pwm1 = pwm_init (&pwm1_cfg);
    pwm2 = pwm_init (&pwm2_cfg);
    pwm3 = pwm_init (&pwm3_cfg);
    pwm4 = pwm_init (&pwm4_cfg);

    uint8_t flash_ticks;

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set (LED2_PIO, PIO_OUTPUT_HIGH);

    //configure Bumper to be NO(normally open) to set bumper_activite to high
    pio_config_set(BUMPER_DETECT, PIO_INPUT_PULLUP);

    //RADIO select changing address 11 -> 100, 10 -> 90, 01 -> 80, 00 -> 70
    pio_config_set(TOP_SW, PIO_INPUT_PULLUP);
    pio_config_set(BOT_SW, PIO_INPUT_PULLUP);

    pacer_init (LOOP_POLL_RATE);
    flash_ticks = 0;
    
    //Start pwm channels
    pwm_channels_start (pwm_channel_mask (pwm1) | pwm_channel_mask (pwm2) | pwm_channel_mask (pwm3) | pwm_channel_mask (pwm4)    );
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
    nrf24_t *nrf;
    spi_t spi;
    usb_cdc_t usb_cdc;
    usb_serial_init(NULL, "/dev/usb_tty");

    freopen("/dev/usb_tty", "a", stdout);
    freopen("/dev/usb_tty", "r", stdin);
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
    int state = 0;
    while (1){
        pacer_wait();

        pio_output_toggle (LED1_PIO);
        if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 1){
            nrf24_set_address(nrf, 100);
        }else if (pio_input_get(TOP_SW) == 1 && pio_input_get(BOT_SW) == 0){
            nrf24_set_address(nrf, 90);
        }else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 1){
            nrf24_set_address(nrf, 80);
        }else if (pio_input_get(TOP_SW) == 0 && pio_input_get(BOT_SW) == 0){
            nrf24_set_address(nrf, 70);
        }
        if(pio_input_get(BUMPER_DETECT)== 0){
            pwm_duty_set(pwm1, 0);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(pwm3, 0);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);//stop
            char buffer1[32];
            sprintf (buffer1, "1\r\n");
            nrf24_write(nrf, buffer1, sizeof (buffer1));
            delay_ms(5000);
            nrf24_listen(nrf);
        }

        if (battery_millivolts() < 6400) {
            pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);
        }else{
            pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);
        }
        printf("%d\n", battery_millivolts());
        //printf("%d\n", pio_input_get(BOT_SW));
        /* Wait until next clock tick.  */

        //LED strip
        if (count++ == NUM_LEDS) {
            // wait for a revolution
            ledbuffer_clear(leds);
            if (blue) {
                ledbuffer_set(leds, 0, 0, 0, 100);
                ledbuffer_set(leds, NUM_LEDS / 2, 0, 0, 100);
            } else {
                ledbuffer_set(leds, 0, 100, 0, 0);
                ledbuffer_set(leds, NUM_LEDS / 2, 100, 0, 0);
            }
            blue = !blue;
            count = 0;
        }

        ledbuffer_write (leds);
        ledbuffer_advance (leds, 1);
        //LED strip

        char buffer[32];
        if (nrf24_read(nrf, buffer, sizeof(buffer))) {
            //printf("%s\n", buffer);
            //printf("%d\n", atoi(&buffer[12]));
            //pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED2_PIO);
            fflush(stdout);
        }

        int duty_1 = atoi(&buffer[4]);
        pwm_duty_set(pwm1, duty_1);
        int dir_1 = atoi(&buffer[0]);
        if (dir_1 == 0){
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
        }else{
            pio_config_set(PWM2_PIO, PIO_OUTPUT_HIGH);
        }
        int duty_2 = atoi(&buffer[9]);
        pwm_duty_set(pwm3, duty_2);
        int dir_2 = atoi(&buffer[2]);
        if (dir_2 == 0){
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);
        }else{
            pio_config_set(PWM4_PIO, PIO_OUTPUT_HIGH);
        }
        //change servo position
        // int servo = atoi(&buffer[15]);
        // set_servo(servo);

        
        

        
    }
        
    
    return 0;
}
