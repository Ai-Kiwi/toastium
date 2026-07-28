#ifndef KERNEL_PROCESS_HANDLER_TRAP_H
#define KERNEL_PROCESS_HANDLER_TRAP_H



#include "types.h"
typedef struct {
    u64 queued_response;
    bool8 skip_instruction;
    bool8 kill_process;
    bool8 send_response;
} interruptable_trap_response;

void handle_async_trap();

#endif