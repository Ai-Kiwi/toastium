#ifndef KERNEL_SYSCALL_UART_H
#define KERNEL_SYSCALL_UART_H

#include "kernel/trap/handler.h"
#include "types.h"

u64 syscall_uart(trap_data *trap, bool8 async);

#endif