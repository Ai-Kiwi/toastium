#ifndef KERNEL_TRAP_INTERRUPTABLE_SYSCALL_H
#define KERNEL_TRAP_INTERRUPTABLE_SYSCALL_H

#include "kernel/trap/handler.h"
#include "kernel/trap/process_handler.h"
#include "types.h"

u64 interruptable_trap_syscall(trap_data *trap, interruptable_trap_response *response);

#endif