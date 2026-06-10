#ifndef KERNEL_RADIX_H
#define KERNEL_RADIX_H

#include "include/types.h"
#include "kernel/memory/allocator.h"

u64 kernel_radix_get_child(u64 addr, u64 key, u8 depth, u8 level_depth);
u64 kernel_radix_create_child(u64 addr, u64 key, u64 child_addr, u8 depth, u8 level_depth);
bool8 kernel_radix_remove_child(u64 addr, u64 key, u8 depth, u8 level_depth);
u64 kernel_radix_create_tree(u8 level_depth);
void kernel_radix_delete(u64 addr, bool8 remove_leaves, u8 depth, u8 level_depth);

#endif