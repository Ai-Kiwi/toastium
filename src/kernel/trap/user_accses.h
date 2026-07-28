#ifndef KERNEL_USER_ACCESS_H
#define KERNEL_USER_ACCESS_H

#include "include/types.h"
#include "kernel/process/process.h"

bool8 kernel_read_user(u64 src, u64 size, u64 dest, process *proc);

#endif