#include "kernel_trap.h"
#include "def.h"

kpid kernel_running_process;

kpid kernel_scheduler_next_process() {
    return null_program_pid;
}

void kernel_schedular_init() {
    kernel_running_process = null_program_pid;
}