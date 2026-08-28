#ifndef KERNEL_SYSCALL_FILE_SYSTEM_H
#define KERNEL_SYSCALL_FILE_SYSTEM_H

#include "kernel/trap/handler.h"
#include "types.h"

#define MAX_OPEN_FILE_DESCRIPTORS 1024

void syscall_fs_open(trap_data *trap);
void syscall_fs_read(trap_data *trap);
void syscall_fs_write(trap_data *trap);
void syscall_fs_seek(trap_data *trap);

#endif