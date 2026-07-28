#include "board.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "kernel/trap/process_handler.h"
#include "types.h"

void interruptable_trap_syscall(trap_data *trap, interruptable_trap_response *response) {
    u64 async_response = syscall_async_handler(trap);
    if (async_response == 1) {
        response->kill_process = TRUE;
    }
    response->skip_instruction = TRUE;
}



