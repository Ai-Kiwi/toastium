#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/timer/timer.h"

kpid kernel_running_process;

kpid kernel_scheduler_next_process() {
    kernel_running_process = null_program_pid;
    kernel_set_timer_future_ms(4);
    return null_program_pid;
}

void kernel_schedular_init() {
    kernel_running_process = null_program_pid;
}