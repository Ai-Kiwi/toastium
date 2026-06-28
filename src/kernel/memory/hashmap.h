#ifndef KERNEL_HASHMAP_H
#define KERNEL_HASHMAP_H

#include "types.h"

typedef struct {
    u64 *start;
    u64 len;
    u64 (*hash_function)(u64);
    bool8 (*equal_function)(u64, u64);
} kernel_hashmap;

typedef struct {
    u64 next_leaf;
    u64 key;
    u64 data;
} __attribute__((aligned(64))) kernel_hashmap_leaf;

typedef enum {
    KHASHMAP_TYPE_STR,
    KHASHMAP_TYPE_NUMBER,
} khashmap_builtin_types;

#endif