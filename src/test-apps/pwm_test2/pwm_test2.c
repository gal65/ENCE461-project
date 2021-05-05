/* File:   pwm_test2.c
   Author: M. P. Hayes, UCECE
   Date:   15 April 2013
   Descr:  This example starts two channels simultaneously; one inverted
           with respect to the other.
*/
#include "pwm.h"
#include "pio.h"

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


int
main (void)
{
    pwm_t pwm1;
    pwm_t pwm2;
    pwm_t pwm3;
    pwm_t pwm4;

    pwm1 = pwm_init (&pwm1_cfg);
    pwm2 = pwm_init (&pwm2_cfg);
    pwm3 = pwm_init (&pwm3_cfg);
    pwm4 = pwm_init (&pwm4_cfg);

    pwm_channels_start (pwm_channel_mask (pwm1) | pwm_channel_mask (pwm2) | pwm_channel_mask (pwm3) | pwm_channel_mask (pwm4)
    );
    pio_config_set(nSLP_PIO, PIO_OUTPUT_HIGH);
 
    while (1){
            //Forward PWM, fast decay
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);
            
            //Forward PWM, slow decay
            //pio_config_set(PWM1_PIO, PIO_OUTPUT_HIGH);
            //pio_config_set(PWM3_PIO, PIO_OUTPUT_HIGH);
            
            //Reverse PWM, fast decay
            //pio_config_set(PWM1_PIO, PIO_OUTPUT_LOW);
            //pio_config_set(PWM3_PIO, PIO_OUTPUT_LOW);

            //Reverse PWM, slow decay
            //pio_config_set(PWM2_PIO, PIO_OUTPUT_HIGH);
            //pio_config_set(PWM4_PIO, PIO_OUTPUT_HIGH);
    }
        
    
    return 0;
}
