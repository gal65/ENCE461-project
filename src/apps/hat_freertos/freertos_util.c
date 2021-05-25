#include "freertos_util.h"

#include "pio.h"
#include "task.h"
#include "target.h"

void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();

    pio_output_set(LED1_PIO, true);
    pio_output_set(LED2_PIO, false);
    while(true);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* name) {
    taskDISABLE_INTERRUPTS();

    pio_output_set(LED1_PIO, false);
    pio_output_set(LED2_PIO, true);
    while(true);
}