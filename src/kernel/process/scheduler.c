#include "arch_vma/virtual_memory.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/timer/timer.h"
#include "types.h"

pid kernel_running_process;

void swap_running_process(kernel_process *process) {
    arch_vma_swap(process);
}

pid kernel_scheduler_next_process() {
    kernel_running_process = null_program_pid;
    kernel_set_timer_future_ms(4);
    return null_program_pid;
}

void kernel_schedular_init() {
    kernel_running_process = null_program_pid;
    //make sure to set process trapframe hart_id to correct value, both kernel and userspace

}

void kernel_schedular_bootstrap(u64 hart_id) {
    kernel_process *idle_process = (kernel_process *)kernel_process_from_id((pid)hart_id);
    if ((u64)idle_process == 0) {
        PANIC("INVALID_IDLE_PROCESS_FOR_HART", (s64)hart_id, (s64)idle_process, 0);
    }

    swap_running_process(idle_process);

    
}