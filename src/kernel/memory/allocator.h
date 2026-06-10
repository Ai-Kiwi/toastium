#ifndef KERNEL_ALLOCATOR_H
#define KERNEL_ALLOCATOR_H

#include "types.h"

void kernel_allocator_init();
u64 kernel_allocator_acquire(u64 size_bytes);
void kernel_allocator_release(u64 location);
u64 kernel_allocator_bump(u64 size);

#endif