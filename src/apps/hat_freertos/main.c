#include "target.h"

#include "pio.h"
#include "delay.h"

#include "freertos_util.h"

#define STACK_SIZE 512

void blink_task(void*);

int main(void) {
    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_HIGH);

    xTaskCreate(blink_task, "blink", STACK_SIZE, NULL, 2, NULL);

    vTaskStartScheduler();
    while(true);
}

void blink_task(void* params) {

    while(true) {
        pio_output_set(LED1_PIO, true);
        vTaskDelay(MS_TO_TICKS(500));
        pio_output_set(LED1_PIO, false);
        vTaskDelay(MS_TO_TICKS(500));
    }
}

void trap(void) {
    taskDISABLE_INTERRUPTS();
    pio_output_set(LED1_PIO, true);
    pio_output_set(LED2_PIO, false);
    while(true);
}