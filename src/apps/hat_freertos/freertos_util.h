#ifndef FREERTOS_UTIL_H
#define FREERTOS_UTIL_H

#include "FreeRTOS.h"
#include "task.h"

void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t task, char* name);

#define MS_TO_TICKS(x) (x) / portTICK_PERIOD_MS

#endif