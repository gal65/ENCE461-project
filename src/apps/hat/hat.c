#include "target.h"

#include "nrf24.h"
#include "pio.h"
#include "pacer.h"
#include "stdio.h"
#include "delay.h"
#include "adc.h"

static void panic(void);

#define ADC_CLOCK_FREQ 24000000

static const adc_cfg_t adc_cfg =
{
    .bits = 12,
    .channel = ADC_CHANNEL_0,
    .trigger = ADC_TRIGGER_SW,
    .clock_speed_kHz = ADC_CLOCK_FREQ / 1000
};

int main (void)
{
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
    adc_t adc;

    pacer_init(10);
    adc = adc_init (&adc_cfg);
    spi = spi_init(&nrf_spi);

    nrf = nrf24_create(spi, RADIO_CE_PIO, RADIO_IRQ_PIO);
    if (!nrf) panic();

    // initialize the NRF24 radio with its unique 5 byte address
    if (!nrf24_begin(nrf, 4, 100, 32)) panic();
    
    uint16_t data[1];
    char buffer[32];
    while(1) {
        pacer_wait();
        adc_read(adc, data, sizeof(data));
        sprintf(buffer, "f: %d b: 0 l: 0 r: 0\n", (int)data[0]);
        nrf24_write(nrf, buffer, sizeof(buffer));
    }
}

static void panic(void)
{
    while (1) {
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
        delay_ms(1000);
    }
}