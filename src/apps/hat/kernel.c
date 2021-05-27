#include "kernel.h"

#include "pit.h"
#include <stdint.h>

void kernel_run(task_t* tasks, int num_tasks)
{
    pit_init();

    while (true) {
        for (int i = 0; i < num_tasks; i++) {
            uint32_t current_time = pit_get();
            if ((pit_get() - tasks[i].last_wakeup) >= tasks[i].period_ms) {
                tasks[i].last_wakeup = current_time;
                tasks[i].func();
                break;
            }
        }
    }
}

task_t create_task(task_func_t func, uint32_t period_ms)
{
    return (task_t) { func, period_ms, 0 };
}