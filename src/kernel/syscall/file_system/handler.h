#ifndef KERNEL_SYSCALL_FILE_SYSTEM_H
#define KERNEL_SYSCALL_FILE_SYSTEM_H

#include "kernel/trap/handler.h"
#include "types.h"

void syscall_fs_open(trap_data *trap);

#endif