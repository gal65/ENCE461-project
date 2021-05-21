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


/* Define how fast ticks occur.  This must be faster than
   TICK_RATE_MIN.  */
enum {LOOP_POLL_RATE = 200};

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


void driving_motor1(int x, pwm_t pwm1, pio_t AN2 , pwm_t pwm3,  pio_t BN2){
    int state = 0;
    if(x == 0){
        state++;
    }
    if(state < 5){
    pwm_duty_set(pwm1, 800);
    pio_config_set(AN2, PIO_OUTPUT_LOW);
    pwm_duty_set(pwm3, 800);
    pio_config_set(BN2, PIO_OUTPUT_LOW);//forward
    }
    if(state>=5 && state<10){
        pwm_duty_set(pwm1, 700);
        pio_config_set(AN2, PIO_OUTPUT_HIGH);
        pwm_duty_set(pwm3, 700);
        pio_config_set(BN2, PIO_OUTPUT_HIGH);//backward

    }
    //if(state>4000 && state < 6000){
        //pwm_duty_set(pwm1, 500);
        //pio_config_set(AN2, PIO_OUTPUT_HIGH);
        //pwm_duty_set(pwm3, 500);
        //pio_config_set(BN2, PIO_OUTPUT_HIGH);

    //}
    if(state>=10){
        pwm_duty_set(pwm1, 0);
        pio_config_set(AN2, PIO_OUTPUT_LOW);
        pwm_duty_set(pwm3, 0);
        pio_config_set(BN2, PIO_OUTPUT_LOW);//stop

    }
    
}

void driving_motor2(int x, pwm_t pwm1, pio_t AN2 , pwm_t pwm3,  pio_t BN2){
    
    if(x > 500){
        pwm_duty_set(pwm1, 800);
        pio_config_set(AN2, PIO_OUTPUT_LOW);
        pwm_duty_set(pwm3, 800);
        pio_config_set(BN2, PIO_OUTPUT_LOW); //forward
    }
    else if(x < -500){
        pwm_duty_set(pwm1, 500);
        pio_config_set(AN2, PIO_OUTPUT_HIGH);
        pwm_duty_set(pwm3, 500);
        pio_config_set(BN2, PIO_OUTPUT_HIGH);//backward
    }else{
        pwm_duty_set(pwm1, 0);
        pio_config_set(AN2, PIO_OUTPUT_LOW);
        pwm_duty_set(pwm3, 0);
        pio_config_set(BN2, PIO_OUTPUT_LOW);//stop
    }
    

}

//paninc function for the radio
static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(1000);
    }
}

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

    uint8_t flash_ticks;

    /* Configure LED PIO as output.  */
    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_LOW);

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
    if (!nrf24_begin(nrf, 4, 100, 32))
        panic();
    if (!nrf24_listen(nrf))
        panic();
    int state = 0;
    while (1){

        
        /* Wait until next clock tick.  */
        pacer_wait ();

        flash_ticks++;
        if (flash_ticks >= LOOP_POLL_RATE / (LED_FLASH_RATE * 2))
        {
            flash_ticks = 0;

            /* Toggle LED.  */
            pio_output_toggle (LED1_PIO);
            pio_output_toggle (LED2_PIO);
        }

        char buffer[32];
        if (nrf24_read(nrf, buffer, sizeof(buffer))) {
            //printf("%s\n", buffer);
            //printf("%d\n", atoi(&buffer[12]));
            //pio_output_toggle(LED2_PIO);
            pio_output_toggle(LED1_PIO);
            fflush(stdout);
        }
        //f: 5 b: 4 l: 3 r: 2
        int f = atoi(&buffer[3]);
        printf("%d\n", f);
        int b = atoi(&buffer[9]);

        if(f>3000){
            pwm_duty_set(pwm1, 800);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(pwm3, 800);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);//forward
        }
        else if(f<1000){
            pwm_duty_set(pwm1, 700);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_HIGH);
            pwm_duty_set(pwm3, 700);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_HIGH);//backward
        }
        else{
            pwm_duty_set(pwm1, 0);
            pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
            pwm_duty_set(pwm3, 0);
            pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);//stop
        }

        // int x = atoi(&buffer[12]);
        // printf("%d\n", x);
        // if(x==1){
        //     state++;
        //     printf("state= %d\n", state);
            
        // }
        // if(state > 2 && state < 5){
        //     pwm_duty_set(pwm1, 800);
        //     pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
        //     pwm_duty_set(pwm3, 800);
        //     pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);//forward
        // }
        // if(state > 5 && state < 8){
        //     pwm_duty_set(pwm1, 800);
        //     pio_config_set(PWM2_PIO, PIO_OUTPUT_HIGH);
        //     pwm_duty_set(pwm3, 800);
        //     pio_config_set(PWM4_PIO, PIO_OUTPUT_HIGH);//backward
        // }
        // if(state > 8){
        //     pwm_duty_set(pwm1, 0);
        //     pio_config_set(PWM2_PIO, PIO_OUTPUT_LOW);
        //     pwm_duty_set(pwm3, 0);
        //     pio_config_set(PWM4_PIO, PIO_OUTPUT_LOW);//stop

        // }


        //driving_motor1(x, pwm1, PWM2_PIO, pwm3, PWM4_PIO);
        //driving_motor2(x, pwm1, PWM2_PIO, pwm3, PWM4_PIO);

    }
        
    
    return 0;
}
