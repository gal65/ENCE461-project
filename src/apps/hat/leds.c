#include "leds.h"
#include "init.h"
#include "stdint.h"

#define DEC_AMOUNT 20
uint8_t r_count = 255;
uint8_t b_count = 0;
uint8_t g_count = 0;
typedef enum {
    RED,
    GREEN,
    BLUE
} dec_color_t;

dec_color_t dec_color = RED;

void ledtape_update(void)
{
    ledbuffer_advance(led_buffer, 1);
    switch (dec_color) {
    case RED:
        r_count -= DEC_AMOUNT;
        g_count += DEC_AMOUNT;
        if (r_count <= DEC_AMOUNT) {
            dec_color = GREEN;
        }
        break;
    case GREEN:
        g_count -= DEC_AMOUNT;
        b_count += DEC_AMOUNT;
        if (g_count <= DEC_AMOUNT) {
            dec_color = BLUE;
        }
        break;
    case BLUE:
        b_count -= DEC_AMOUNT;
        r_count += DEC_AMOUNT;
        if (b_count <= DEC_AMOUNT) {
            dec_color = RED;
        }
        break;
    }
    ledbuffer_set(led_buffer, 0, r_count, g_count, b_count);
    ledbuffer_write(led_buffer);
}

void blink_task(void)
{
    pio_output_toggle(LED1_PIO);
}