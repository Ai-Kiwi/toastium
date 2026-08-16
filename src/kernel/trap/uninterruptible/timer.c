#include "timer.h"
#include "kernel/safety/panic.h"
#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_timer(trap_data *trap) {
    if (trap->privilege != TRAP_MODE_SUPERVISOR) {
        PANIC("KERNEL_TRAP_TIMER_NON_SUPERVISOR_PRIVILEGE", trap->privilege,
              trap->fault_addr, trap->fault_pc);
    }

    trap_change_process(trap);
    return 0;
}
