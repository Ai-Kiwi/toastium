#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "kernel/process/process.h"

#define SYSCALL_UART_PRINT 1

u64 kernel_syscall_sync_handler(kernel_trap_data *trap);
u64 kernel_syscall_async_handler(kernel_trap_data *trap);

#endif