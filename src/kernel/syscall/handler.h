#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "kernel/process/process.h"

u64 kernel_syscall_sync_handler(kernel_trap_data *trap);

#endif