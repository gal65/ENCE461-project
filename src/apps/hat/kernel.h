#ifndef KERNEL_H
#define KERNEL_H

#include <stdbool.h>
#include <stdint.h>

#define KERNEL_TICK_FREQUENCY F_CPU

typedef void (*task_func_t)(void);

typedef struct {
    char* name;
    task_func_t func;
    uint32_t period_ms;
    uint32_t last_wakeup;
    bool enabled;
} task_t;

void kernel_init(task_t* tasks_vec, int num_tasks);
void kernel_run(void);

task_t create_task(char* name, task_func_t func, uint32_t period_ms);

void enable_task(char* name);
void disable_task(char* name);

#endif