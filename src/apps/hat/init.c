#include "init.h"

#include "adc.h"
#include "config.h"
#include "delay.h"
#include "ledbuffer.h"
#include "mpu9250.h"
#include "nrf24.h"
#include "sound.h"
#include "usb_serial.h"

#define ADC_CLOCK_FREQ 24000000

#define USE_TEST_CHANNEL 0
#define NRF_CHANNEL 15

uint64_t RADIO_ADDRESSES[] = { 70, 80, 90, 100 };

spi_cfg_t nrf_spi_cfg = {
    .channel = 0,
    .clock_speed_kHz = 1000,
    .cs = RADIO_CS_PIO,
    .mode = SPI_MODE_0,
    .cs_mode = SPI_CS_MODE_FRAME,
    .bits = 8,
};

static const adc_cfg_t adc_cfg_1 = {
    .bits = 12,
    .channel = ADC_CHANNEL_0,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static const adc_cfg_t adc_cfg_2 = {
    .bits = 12,
    .channel = ADC_CHANNEL_1,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

static const adc_cfg_t battery_voltage_adc_cfg = {
    .bits = 12,
    .channel = ADC_CHANNEL_3,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

#if USB_DEBUG
static usb_serial_cfg_t usb_serial_cfg = {
    .read_timeout_us = 1,
    .write_timeout_us = 1,
};
#endif

static twi_cfg_t mpu_twi_cfg = {
    .channel = TWI_CHANNEL_0,
    .period = TWI_PERIOD_DIVISOR(100000), // 100 kHz
    .slave_addr = 0
};

// public:
nrf24_t* nrf = NULL;
adc_t joystick_x_adc = NULL;
adc_t joystick_y_adc = NULL;
adc_t battery_voltage_adc = NULL;
mpu_t* imu = NULL;
ledbuffer_t* led_buffer = NULL;
twi_t imu_twi = NULL;
spi_t nrf_spi = NULL;
// tweeter_t tweeter = NULL;

// #define TWEETER_POLL_RATE 40000
// tweeter_scale_t scale_table[] = TWEETER_SCALE_TABLE(TWEETER_POLL_RATE);

#define NUM_LEDS 26

void radio_configuration(nrf24_t** out_nrf, spi_t* out_spi);
static void panic(void);

void init_hat(void)
{
    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(EXT_STATUS1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(EXT_STATUS2_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(EXT_STATUS3_PIO, PIO_OUTPUT_HIGH);

    pio_config_set(BUTTON_PIO, PIO_PULLUP);
    pio_config_set(JOYSTICK_BUTTON_PIO, PIO_PULLUP);
    pio_config_set(EXT_BUTTON1_PIO, PIO_PULLUP);
    pio_config_set(EXT_BUTTON2_PIO, PIO_PULLUP);
    pio_config_set(PSEUDO_RANDOM_PIO, PIO_INPUT);

    // sound_init();

    radio_configuration(&nrf, &nrf_spi);

#if USB_DEBUG
    // Create non-blocking tty device for USB CDC connection.
    usb_serial_init(&usb_serial_cfg, "/dev/usb_tty");

    freopen("/dev/usb_tty", "a", stdout);
    freopen("/dev/usb_tty", "r", stdin);
#endif

    imu_twi = twi_init(&mpu_twi_cfg);
    imu = mpu9250_create(imu_twi, MPU_ADDRESS);

    joystick_x_adc = adc_init(&adc_cfg_1);
    joystick_y_adc = adc_init(&adc_cfg_2);
    battery_voltage_adc = adc_init(&battery_voltage_adc_cfg);

    led_buffer = ledbuffer_init(LEDTAPE_PIO, NUM_LEDS);
    tweet_sound_init();
}

void radio_configuration(nrf24_t** out_nrf, spi_t* out_spi)
{
    *out_spi = spi_init(&nrf_spi_cfg);

    *out_nrf = nrf24_create(*out_spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!(*out_nrf))
        panic();

    pio_config_set(RADIO_JUMPER_1_PIO, PIO_PULLUP);
    pio_config_set(RADIO_JUMPER_2_PIO, PIO_PULLUP);

    int addr_index = (pio_input_get(RADIO_JUMPER_1_PIO) << 1) | pio_input_get(RADIO_JUMPER_2_PIO);
    addr_index = 3;

#if USE_TEST_CHANNEL
    if (!nrf24_begin(*out_nrf, 4, 0x0123456789, 32))
        panic();
#else
    if (!nrf24_begin(*out_nrf, 15, RADIO_ADDRESSES[addr_index], 32))
        panic();
#endif
}

static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(500);
    }
}