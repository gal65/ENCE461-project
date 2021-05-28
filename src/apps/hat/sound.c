#include "sound.h"

#include "config.h"
#include "dac.h"
#include "pio.h"

uint16_t buffer[4096] = { 0 };

dac_cfg_t single_channel_dac_config = {
    .channel = DAC_CHANNEL_0,
    // default: only enable ch 0
    .channels = BIT(DAC_CHANNEL_0),
    .bits = 12,
    .trigger = DAC_TRIGGER_SW,
    .clock_speed_kHz = 10,
    // disable refresh
    .refresh_clocks = 0,
};

dac_t dac = NULL;

void sound_init(void)
{
    // pio_config_set(SPEAKER_PIO, PIO_OUTPUT_LOW);
    dac = dac_init(&single_channel_dac_config);
    for (int i = 0; i < 4096; i++) {
        buffer[i] = i;
    }

    // for(int i = 128; i < 256; i++) {
    // buffer[i] = 4096 - (255 - i)*16;
    // }
    //

    dac_enable(dac);
    while (true) {
        sound_play();
    }
    dac_disable(dac);
}

void sound_play(void)
{
    dac_write(dac, buffer, sizeof(buffer));
}