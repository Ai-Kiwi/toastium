#ifndef KERNEL_TRAP_UNINTERRUPTIBLE_SYSCALL_H
#define KERNEL_TRAP_UNINTERRUPTIBLE_SYSCALL_H

#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_syscall(trap_data *trap);

#endif