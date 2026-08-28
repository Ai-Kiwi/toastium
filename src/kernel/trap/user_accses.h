#ifndef KERNEL_USER_ACCESS_H
#define KERNEL_USER_ACCESS_H

#include "include/types.h"
#include "kernel/process/process.h"

bool8 kernel_read_user(u64 src, u64 size, void *dest, process *proc);
bool8 kernel_write_user(u64 dest, u64 size, void *src, process *proc);

#endif