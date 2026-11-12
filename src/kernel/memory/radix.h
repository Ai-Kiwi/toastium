#ifndef KERNEL_RADIX_H
#define KERNEL_RADIX_H

#include "include/types.h"

typedef struct {
    void *addr;
    u8 levels;
    u8 level_size;
} radix_tree;

u64 radix_get(radix_tree *tree, u64 key);
// returns old address
u64 radix_insert(radix_tree *tree, u64 key, u64 value);
bool8 radix_remove(radix_tree *tree, u64 key);
void radix_create(radix_tree *output_tree, u8 level_size, u8 levels);
void radix_delete(radix_tree *tree, bool8 remove_leaves);
void radix_iter(radix_tree *tree, u64 parameters, void (*function)(u64, u64));

#endif