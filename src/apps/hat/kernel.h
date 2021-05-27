#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

#define KERNEL_TICK_FREQUENCY F_CPU

typedef void (*task_func_t)(void);

typedef struct {
    task_func_t func;
    uint32_t period_ms;
    uint32_t last_wakeup;
} task_t;

void kernel_run(task_t* tasks, int num_tasks);

task_t create_task(task_func_t func, uint32_t period_ms);

#endif