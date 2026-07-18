#ifndef KERNEL_ALLOCATOR_H
#define KERNEL_ALLOCATOR_H

#include "types.h"

void allocator_init();
u64 mem_alloc(u64 size_bytes);
void mem_free(u64 location);
u64 bump_alloc(u64 size);

#endif