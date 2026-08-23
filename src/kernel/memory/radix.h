#ifndef KERNEL_RADIX_H
#define KERNEL_RADIX_H

#include "include/types.h"

u64 radix_get(u64 addr, u64 key, u8 depth, u8 level_depth);
// returns old address
u64 radix_insert(u64 addr, u64 key, u64 child_addr, u8 depth, u8 level_depth);
bool8 radix_remove(u64 addr, u64 key, u8 depth, u8 level_depth);
u64 radix_create(u8 level_depth);
void radix_delete(u64 addr, bool8 remove_leaves, u8 depth, u8 level_depth);
void radix_iter(u64 addr, u8 depth, u8 depth_size, u64 parameters,
                void (*function)(u64, u64));

#endif