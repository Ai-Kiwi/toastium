#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "kernel/trap/handler.h"
#include "types.h"
#define SYSCALL_UART_PRINT 1

u64 syscall_sync_handler(trap_data *trap);
u64 syscall_async_handler(trap_data *trap);

#endif