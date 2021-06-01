#include <stdlib.h>

#include "adc.h"
#include "common.h"
#include "config.h"
#include "control.h"
#include "delay.h"
#include "init.h"
#include "kernel.h"
#include "leds.h"
#include "mcu_sleep.h"
#include "mmelody.h"
#include "mpu9250.h"
#include "nrf24.h"
#include "pio.h"
#include "pwm.h"
#include "sleep.h"
#include "sound.h"
#include "spi.h"
#include "stdio.h"
#include "twi.h"

void check_bumber_task(void)
{
    char buffer[32];
    if (nrf24_is_data_ready(nrf)) {
        nrf24_read(nrf, buffer, sizeof(buffer));
        if (buffer[0] == '1') {
            tweet_sound_play();
        }
#if USB_DEBUG
        printf("BUMPER\n");
        fflush(stdout);
#endif
    }
}

void enter_low_power(void)
{
    ledbuffer_clear(led_buffer);
    ledbuffer_write(led_buffer);
    disable_task("leds");
}

void exit_low_power(void)
{
    enable_task("leds");
}

void battery_voltage_task(void)
{
#if LOW_POWER_FEATURE
    uint16_t bat;
    adc_read(battery_voltage_adc, &bat, sizeof(bat));

    if (bat < 2715) {
        pio_output_low(LED2_PIO);
        enter_low_power();
    } else {
        pio_output_high(LED2_PIO);
        exit_low_power();
    }
#endif
}

mcu_sleep_cfg_t sleep_cfg = {
    .mode = MCU_SLEEP_MODE_WAIT
};

mcu_sleep_wakeup_cfg_t wakeup_cfg = {
    .pio = BUTTON_PIO,
    .active_high = false
};

void check_sleep_mode_task(void)
{
    static bool prev_button_state = true;

    bool button_state = pio_input_get(BUTTON_PIO);

    if (prev_button_state && !button_state) {
        spi_shutdown(nrf_spi);
        pio_output_low(RADIO_CE_PIO);
        twi_shutdown(imu_twi);
        enter_low_power();
        delay_ms(1000);
        mcu_sleep_wakeup_set(&wakeup_cfg);
        mcu_sleep(&sleep_cfg);
        exit_low_power();
    }
    prev_button_state = button_state;
}

int main(void)
{
    init_hat();
    pio_config_set(EXT_STATUS1_PIO, PIO_OUTPUT_LOW);
    pio_config_set(EXT_STATUS2_PIO, PIO_OUTPUT_LOW);
    pio_config_set(EXT_STATUS3_PIO, PIO_OUTPUT_LOW);

    task_t tasks[] = {
        create_task("battery", battery_voltage_task, 5000),
        create_task("gen_key", generate_key_task, 500),
        create_task("blink", blink_task, 500),
        create_task("leds", ledtape_update, 100),
        create_task("sleep", check_sleep_mode_task, 100),
        create_task("bumper", check_bumber_task, 100),
        create_task("change_control", change_control_method_task, 20),
        create_task("imu_control", imu_control_task, 20),
        create_task("joystick_control", joystick_control_task, 20),
    };

    kernel_init(tasks, sizeof(tasks) / sizeof(task_t));

    disable_task("joystick_control");
    kernel_run();
}