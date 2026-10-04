#include "syscall.h"
#include "kernel/process/process.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "layout.h"
#include "types.h"

u64 uninterruptible_trap_syscall(trap_data *trap) {
    u64 response = syscall_sync_handler(trap);
    if (response == TRAP_UNHANDLED) {
        // needs to be handled by async
        return TRAP_UNHANDLED;
    }
    trap_data_set_response(trap);
    trap_data_iter_instruction(trap);
    return TRAP_HANDLED;
}
