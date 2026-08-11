#include "board.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "types.h"
#include "syscall.h"

u64 uninterruptible_trap_syscall(trap_data *trap) {
    u64 response = syscall_sync_handler(trap);
    if (response == 1) {//process needs to be killed
        kill_process(((process *)trap->process_ptr)->process_id);

        trap_change_process(trap);
        return 0;
    }
    if (response > 0) {
        //needs to be handled by async
        return PROCESS_KERNEL_STACK_TOP;
    }
    trap_data_set_response(trap);
    trap_data_iter_instruction(trap);
    return 0;
}



