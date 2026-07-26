#ifndef KERNEL_SYSCALL_UART_H
#define KERNEL_SYSCALL_UART_H

#include "types.h"
#include "kernel/process/process.h"

u64 syscall_uart(trap_data *trap, bool8 async);

#endif