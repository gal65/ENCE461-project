#include "target.h"

#include "nrf24.h"
#include "pio.h"
#include "fcntl.h"
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
#include "mcu_sleep.h"
#include "tweeter.h"
#include "control_mapping.h"


void radio_configuration(nrf24_t** out_nrf, spi_t* out_spi);

uint64_t RADIO_ADDRESSES[] = {70, 80, 90, 100};

static void panic(void);

#define ADC_CLOCK_FREQ 24000000

spi_cfg_t nrf_spi = {
    .channel = 0,
    .clock_speed_kHz = 1000,
    .cs = RADIO_CS_PIO,
    .mode = SPI_MODE_0,
    .cs_mode = SPI_CS_MODE_FRAME,
    .bits = 8,
};

static const adc_cfg_t adc_cfg_1 =
{
    .bits = 12,
    .channel = ADC_CHANNEL_0,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static const adc_cfg_t adc_cfg_2 =
{
    .bits = 12,
    .channel = ADC_CHANNEL_1,
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

int32_t min(int32_t a, int32_t b) {
    return a < b ? a : b;
}

void blink1_task(void) {
    pio_output_toggle(LED1_PIO);
}

void blink2_task(void) {
    pio_output_toggle(LED2_PIO);
}

//static tweeter_scale_t scale_table[] = TWEETER_SCALE_TABLE (1000);

//static const tweeter_private_t tweety =
//{
  //  .note_clock = 1000,
  //  .note_period = 200,
  //  .note_duty = 200,
  //  .note_holdoff = 50e-3,
  //  .poll_rate = 1000,
  //  .scale_table = &scale_table,
//};

//static const mcu_sleep_wakeup_cfg_t wake_up_pin=
//{
//    .pio = PA2_PIO,
//    .active_high = false
//};

//static const mcu_sleep_cfg_t sleepy_mode =
//{
//    .mode = MCU_SLEEP_MODE_BACKUP
//};

int main (void)
{
    nrf24_t* nrf = NULL;
    spi_t spi = NULL;
    adc_t adc_1;
    adc_t adc_2;


    pio_config_set (LED1_PIO, PIO_OUTPUT_LOW);
    pio_config_set (LED2_PIO, PIO_OUTPUT_LOW);
    pio_config_set(BUTTON_PIO, PIO_PULLUP);

    // task_t tasks[] = {
        // create_task(blink1_task, 400),
        // create_task(blink2_task, 500)
    // };

    pio_output_toggle(LED1_PIO);
    // kernel_run(tasks, 2);

    radio_configuration(&nrf, &spi);

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
    adc_1 = adc_init (&adc_cfg_1);
    adc_2 = adc_init (&adc_cfg_2);

    //tweeter_init(&tweety, 1000, &scale_table);

    uint16_t data;
    while(1) {
        pacer_wait();
        //tweeter_update(tweety);
        adc_read(adc_1, &data, sizeof(data));
        adc_read(adc_2, &data, sizeof(data));
        sprintf(adc_1, "\n");
        sprintf(adc_2, "\n");
        // sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
        // nrf24_write(nrf, buffer, sizeof(buffer));
        //if (pio_input_get(BUTTON_PIO))
        //{
        //    delay_ms(1000);
        //    mcu_sleep_wakeup_set(&wake_up_pin);
        //    mcu_sleep(&sleepy_mode);
        //}
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
                    motor_data_t move = get_motor_values_imu(accel);
                    // nrf24_write(nrf, &move, sizeof(move));
                    char buffer[32] = {0};
                    snprintf(buffer, sizeof(buffer), "%d %d %-4lu %-4lu",
                        move.left_motor_direction,
                        move.right_motor_direction,
                        move.left_motor_pwm,
                        move.right_motor_pwm
                    );
                    printf("%s\n", buffer);
                    nrf24_write(nrf, buffer, sizeof(buffer));
                    pio_output_toggle(LED1_PIO);
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



void radio_configuration(nrf24_t** out_nrf, spi_t* out_spi) {
    *out_spi = spi_init(&nrf_spi);

    *out_nrf = nrf24_create(*out_spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!(*out_nrf)) panic();

    pio_config_set(RADIO_JUMPER_1_PIO, PIO_PULLDOWN);
    pio_config_set(RADIO_JUMPER_1_PIO, PIO_PULLDOWN);

    int addr_index = (pio_input_get(RADIO_JUMPER_1_PIO) << 1) | pio_input_get(RADIO_JUMPER_2_PIO);
    addr_index = 3;

    // initialize the NRF24 radio with its unique 5 byte address
    // if (!nrf24_begin(nrf, 4, RADIO_ADDRESSES[addr_index], 32)) panic();
    if (!nrf24_begin(*out_nrf, 4, 100, 32)) panic();
}

static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(500);
    }
}