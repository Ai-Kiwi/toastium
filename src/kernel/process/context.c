#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/process/scheduler.h"

void change_context(kernel_process *process) {
    arch_vma_swap(process);
}

//handles old process cleanup as well
void kernel_context_change_process(kernel_process *process, u64 hart_id) {
    kernel_process *old_process = (kernel_process *)((arch_trapframe *)TRAPFRAME_ADDRESS)->process_ptr;

    change_context(process);
    process->running = TRUE;
    process->page_fault_trap_frame->hart_id = hart_id;
    process->kernelspace_trap_frame->hart_id = hart_id;
    process->userspace_trap_frame->hart_id = hart_id;

    //cleanup the old process
    switch (old_process->process_type) {
        case KPROC_TYPE_NORMAL:
            old_process->running = FALSE;

            if (old_process->block_waiting == 0) {
                kernel_scheduler_queue_process(old_process);
            }
            break;
        case KPROC_TYPE_IDLE:
            break;
        case KPROC_TYPE_DEAD:
            kernel_process_cleanup_process(old_process);
            break;
        default:
            PANIC("UNHANDLED_PROCESS_TYPE", process->process_type, process->process_id, 0);
            break;
    }
}

void kernel_context_bootstrap(u64 hart_id) {
    kernel_process *idle_process = (kernel_process *)kernel_process_from_id((pid)hart_id);
    if ((u64)idle_process == 0) {
        PANIC("INVALID_IDLE_PROCESS_FOR_HART", (s64)hart_id, (s64)idle_process, 0);
    }

    change_context(idle_process);
}