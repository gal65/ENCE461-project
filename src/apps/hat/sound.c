#include "sound.h"

#include "config.h"
#include "dac.h"
#include "delay.h"
#include "mmelody.h"
#include "pio.h"
#include "tweeter.h"

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

#define POLL_RATE 20000
#define TWEETER_TASK_RATE 20000
#define TUNE_TASK_RATE 200
#define TUNE_BPM_RATE 255

static tweeter_t tweeter;
static mmelody_t melody;
static mmelody_obj_t melody_info;
static tweeter_obj_t tweeter_info;

// squeaker_scale_t scale_table[] = SQUEAKER_SCALE_TABLE(LOOP_POLL_RATE);
static const char tune1[] = {
#include "sound.mmel"
    "          :"
};

void tweet_sound_init(void)
{
    static tweeter_scale_t scale_table[] = TWEETER_SCALE_TABLE(TWEETER_TASK_RATE);
    tweeter = tweeter_init(&tweeter_info, TWEETER_TASK_RATE, scale_table);
    pio_config_set(EXT_BUZZER_PIO, PIO_OUTPUT_LOW);
    melody = mmelody_init(&melody_info, TUNE_TASK_RATE,
        (mmelody_callback_t)tweeter_note_play, tweeter);

    mmelody_speed_set(melody, TUNE_BPM_RATE);

    mmelody_play(melody, tune1);
}

static void tweeter_task(void)
{
    pio_output_set(EXT_BUZZER_PIO, tweeter_update(tweeter));
}

static void tune_task(void)
{
    mmelody_update(melody);
}

void tweet_sound_play(void)
{
    int ticks_per_melody = TWEETER_TASK_RATE / TUNE_TASK_RATE;

    // for (int i = 0; i < 158000; i++) {
    for (int i = 0; i < 50000; i++) {
        DELAY_US(1000000 / TWEETER_TASK_RATE);
        tweeter_task();
        if (!(i % ticks_per_melody)) {
            tune_task();
        }
    }
}
