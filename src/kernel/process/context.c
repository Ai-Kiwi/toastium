#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/process/scheduler.h"

void change_context(process *process) {
    vma_swap(process);
}

//handles old process cleanup as well
void context_change_process(process *proc, u64 hart_id) {
    process *old_proc = (process *)((trapframe *)TRAPFRAME_ADDRESS)->process_ptr;

    change_context(proc);
    proc->running = TRUE;
    proc->page_fault_trapframe->hart_id = hart_id;
    proc->kernelspace_trapframe->hart_id = hart_id;
    proc->userspace_trapframe->hart_id = hart_id;

    //cleanup the old process
    switch (old_proc->process_type) {
        case PROC_TYPE_NORMAL:
            old_proc->running = FALSE;

            if (old_proc->block_waiting == 0) {
                scheduler_queue_process(old_proc);
            }
            break;
        case PROC_TYPE_IDLE:
            break;
        case PROC_TYPE_DEAD:
            process_cleanup(old_proc);
            break;
        default:
            PANIC("UNHANDLED_PROCESS_TYPE", proc->process_type, proc->process_id, 0);
            break;
    }
}

void context_bootstrap(u64 hart_id) {
    process *idle_process = (process *)process_from_id((pid)hart_id);
    if ((u64)idle_process == 0) {
        PANIC("INVALID_IDLE_PROCESS_FOR_HART", (s64)hart_id, (s64)idle_process, 0);
    }

    change_context(idle_process);
}