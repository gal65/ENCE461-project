#include "common.h"
#include "delay.h"
#include "pio.h"
#include "pwm.h"

#include <stdio.h>
// #include "nrf24.h"

// motor_data_t read_movement_data(nrf24_t nrf) {
//     static motor_data_t data;
//     nrf24_read(nrf, &data, sizeof(motor_data_t));
//     return data;
// }

void print_movement_data(motor_data_t data)
{
    printf("left: %lu %s, right %lu %s\n",
        data.left_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD",
        data.right_motor_pwm,
        data.left_motor_direction == FORWARD ? "FORWARD" : "BACKWARD");
}

// positino 0 -> 255
#define SERVO_MIN_US 600
#define SERVO_MAX_US 2800
#define SERVO_PWM_PERIOD_US 20000

void write_servo_bitbang(pio_t pio, uint8_t position)
{
    int us = position * 2200 / 255 + 600;
    int rest = 20000 - us;
    for (int i = 0; i < 5; i++) {
        pio_output_high(pio);
        DELAY_US(us);
        pio_output_low(pio);
        DELAY_US(rest);
    }
}

#define PWM_FREQ_HZ 50
static pwm_cfg_t servo_pwm_cfg = {
    .pio = 0,
    .period = PWM_PERIOD_DIVISOR(PWM_FREQ_HZ),
    .duty = PWM_DUTY_DIVISOR(PWM_FREQ_HZ, 0),
    .align = PWM_ALIGN_LEFT,
    .polarity = PWM_POLARITY_HIGH,
    .stop_state = PIO_OUTPUT_LOW
};

pwm_t servo_pwm;

void init_servo(pio_t pio)
{
    servo_pwm_cfg.pio = pio;
    servo_pwm = pwm_init(&servo_pwm_cfg);
    pwm_channels_start(pwm_channel_mask(servo_pwm));
}

#define TO_CPU_CLKS(MS) ((MS)*F_CPU / 1000)

#define SERVO_MIN_DUTY_CLKS TO_CPU_CLKS(SERVO_MIN_DUTY_MS)
#define SERVO_MAX_DUTY_CLKS TO_CPU_CLKS(SERVO_MAX_DUTY_MS)

void set_servo(uint8_t position)
{
    uint32_t duty_clks = ((uint32_t)position) * (SERVO_MAX_DUTY_CLKS - SERVO_MIN_DUTY_CLKS) / 255 + SERVO_MIN_DUTY_CLKS;
    pwm_duty_set(servo_pwm, duty_clks);
}