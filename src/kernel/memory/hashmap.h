#ifndef KERNEL_HASHMAP_H
#define KERNEL_HASHMAP_H

#include "types.h"

typedef struct {
    u64 *start;
    u64 len; //length of data (stores as 4 bytes per entry so len/4 is byte len)
    u64 (*hash_function)(u64);
    bool8 (*equal_function)(u64, u64);
} hashmap;

typedef struct {
    u64 next_leaf;
    u64 key;
    u64 data;
} __attribute__((aligned(64))) hashmap_leaf;

typedef enum {
    HASHMAP_TYPE_STR,
    HASHMAP_TYPE_NUMBER,
} hashmap_types;

u64 hashmap_insert(hashmap *hashmap, u64 key, u64 data);
u64 hashmap_remove(hashmap *hashmap, u64 key);
u64 hashmap_get(hashmap *hashmap, u64 key);
void hashmap_create(hashmap_types type, hashmap *init_hashmap);

#endif