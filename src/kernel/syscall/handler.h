#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "kernel/trap/handler.h"
#include "types.h"
#define SYSCALL_UART_PRINT 1
#define SYSCALL_IDLE_HANG 3
#define SYSCALL_FS_OPEN 4

u64 syscall_sync_handler(trap_data *trap);
u64 syscall_async_handler(trap_data *trap);

#endif