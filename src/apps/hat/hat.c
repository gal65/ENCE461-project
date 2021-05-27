#include "target.h"

#include "nrf24.h"
#include <pio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "pacer.h"
#include "stdio.h"
#include "delay.h"
#include "adc.h"
#include "usb_serial.h"
#include "mpu9250.h"
#include "config.h"
#include "common.h"
#include "kernel.h"

void radio_configuration(nrf24_t* nrf, spi_t* spi);

uint64_t RADIO_ADDRESSES[] = {70, 80, 90, 100};

static void panic(void);

#define ADC_CLOCK_FREQ 24000000

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channel = ADC_CHANNEL_0,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static usb_serial_cfg_t usb_serial_cfg =
{
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};

static twi_cfg_t mpu_twi_cfg =
{
    .channel = TWI_CHANNEL_0,
    .period = TWI_PERIOD_DIVISOR(100000), // 100 kHz
    .slave_addr = 0
};

int16_t min(int16_t a, int16_t b) {
    return a < b ? a : b;
}

void blink1_task(void) {
    pio_output_toggle(LED1_PIO);
}

void blink2_task(void) {
    pio_output_toggle(LED2_PIO);
}

int main (void)
{
    nrf24_t *nrf = NULL;
    spi_t* spi = NULL;
    adc_t adc;

    movement_data_t movement_data;    

    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_LOW);

    task_t tasks[] = {
        create_task(blink1_task, 400),
        create_task(blink2_task, 500)
    };

    pio_output_toggle(LED1_PIO);
    kernel_run(tasks, 2);

    radio_configuration(nrf, spi);

    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init (&usb_serial_cfg, "/dev/usb_tty");
    uint8_t flash_ticks = 0;

    freopen ("/dev/usb_tty", "a", stdout);
    freopen ("/dev/usb_tty", "r", stdin);

    // Initialise the TWI (I2C) bus for the MPU
    twi_t twi_mpu = twi_init (&mpu_twi_cfg);
    // Initialise the MPU9250 IMU
    mpu_t* mpu = mpu9250_create (twi_mpu, MPU_ADDRESS);

    pacer_init(10);
    adc = adc_init (&adc_cfg);

    
    uint16_t data;
    while(1) {
        pacer_wait();
        adc_read(adc, &data, sizeof(data));
        // sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
        // nrf24_write(nrf, buffer, sizeof(buffer));
        if (mpu)
        {
            /* read in the accelerometer data */
            if (! mpu9250_is_imu_ready (mpu))
            {
                printf("Waiting for IMU to be ready...\n");
            }
            else
            {
                int16_t accel[3];
                if (mpu9250_read_accel (mpu, accel))
                {
                    int16_t abs_y = abs(accel[1]);
                    if(abs_y < 1000) {
                        movement_data.left_motor_direction = FORWARD;
                        movement_data.right_motor_direction = FORWARD;
                        movement_data.left_motor_pwm = 0;
                        movement_data.right_motor_pwm = 0;
                    }
                    else {
                        movement_data.left_motor_direction = accel[1] > 0 ? FORWARD : BACKWARD;
                        movement_data.right_motor_direction = accel[1] > 0 ? FORWARD : BACKWARD;
                        printf("%d.....", abs_y);
                        movement_data.left_motor_pwm = (min(abs_y, 8000) - 1000) / 7;
                        movement_data.right_motor_pwm = (min(abs_y, 8000) - 1000) / 7;
                    }
                    print_movement_data(movement_data);
                    if(movement_data.left_motor_direction == BACKWARD) {
                        movement_data.left_motor_pwm = 1000 - movement_data.left_motor_pwm;
                    }
                    if(movement_data.right_motor_direction == BACKWARD) {
                        movement_data.right_motor_pwm = 1000 - movement_data.right_motor_pwm;
                    }
                    // nrf24_write(nrf, &movement_data, sizeof(movement_data));
                    char buffer[] = "test\n";
                    nrf24_write(nrf, buffer, sizeof(buffer));
                }
                else
                {
                    printf("ERROR: failed to read acceleration\n");
                }
            }
        }
        else
        {
            printf("ERROR: can't find MPU9250!\n");
        }

        fflush(stdout);
    }
}

void radio_configuration(nrf24_t* nrf, spi_t* spi) {
    spi_cfg_t nrf_spi = {
        .channel = 0,
        .clock_speed_kHz = 1000,
        .cs = RADIO_CS_PIO,
        .mode = SPI_MODE_0,
        .cs_mode = SPI_CS_MODE_FRAME,
        .bits = 8,
    };

    *spi = spi_init(&nrf_spi);

    nrf = nrf24_create(*spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf) panic();

    pio_config_set(RADIO_JUMPER_1_PIO, PIO_PULLDOWN);
    pio_config_set(RADIO_JUMPER_1_PIO, PIO_PULLDOWN);

    int addr_index = (pio_input_get(RADIO_JUMPER_1_PIO) << 1) | pio_input_get(RADIO_JUMPER_2_PIO);

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 4, RADIO_ADDRESSES[addr_index], 32)) panic();
}

static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(1000);
    }
}