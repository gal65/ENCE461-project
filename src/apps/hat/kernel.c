#include "kernel.h"

#include "pit.h"
#include <stdint.h>
#include <string.h>

task_t* tasks = NULL;
int tasks_len;

void kernel_init(task_t* tasks_vec, int num_tasks)
{
    tasks = tasks_vec;
    tasks_len = num_tasks;
}

void kernel_run(void)
{
    pit_init();

    while (true) {
        for (int i = 0; i < tasks_len; i++) {
            uint32_t current_time = pit_get();
            if ((pit_get() - tasks[i].last_wakeup) >= tasks[i].period_ms && tasks[i].enabled) {
                tasks[i].last_wakeup = current_time;
                tasks[i].func();
                break;
            }
        }
    }
}

task_t create_task(char* name, task_func_t func, uint32_t period_ms)
{
    return (task_t) { name, func, period_ms, 0, true };
}

void enable_task(char* name)
{
    for (int i = 0; i < tasks_len; i++) {
        if (!strcmp(tasks[i].name, name)) {
            tasks[i].enabled = true;
        }
    }
}
void disable_task(char* name)
{
    for (int i = 0; i < tasks_len; i++) {
        if (!strcmp(tasks[i].name, name)) {
            tasks[i].enabled = false;
        }
    }
}