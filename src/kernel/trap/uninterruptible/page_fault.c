#include "board.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_page_fault(trap_data *trap, process_trap_state past_proc_state) {
    process *proc = (process *)trap->process_ptr;

    if (past_proc_state != PROC_TRAP_PROCESS && proc->reading_userspace == FALSE) {
        PANIC("KERNEL_TRAP_PAGE_FAULT",trap->privilege, trap->fault_addr, trap->fault_pc);
    }

    if (past_proc_state == PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP) {
        u64 stack_location = trapframe_stack_ptr(proc->kernelspace_trapframe);
        stack_location = ROUND_MOD_DOWN(stack_location - 8, 8);
        return stack_location;
    }else{
        return PROCESS_KERNEL_STACK_TOP;
    }
}



