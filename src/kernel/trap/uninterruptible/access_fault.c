#include "board.h"
#include "def.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "types.h"
#include "access_fault.h"

u64 uninterruptible_trap_access_fault(trap_data *trap, process_trap_state past_proc_state) {
    if (past_proc_state != PROC_TRAP_PROCESS || trap->privilege != TRAP_MODE_USER) {
        PANIC("KERNEL_TRAP_ACCESS_FAULT",trap->privilege, trap->fault_addr, trap->fault_pc);
    }

    kill_process(((process *)trap->process_ptr)->process_id);
    trap_change_process(trap);
    return 0;
}



