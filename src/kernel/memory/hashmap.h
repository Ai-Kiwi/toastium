#ifndef KERNEL_HASHMAP_H
#define KERNEL_HASHMAP_H

#include "types.h"

typedef struct hashmap_leaf hashmap_leaf;
struct hashmap_leaf {
    hashmap_leaf *next_leaf;
    u64 key;
    u64 data;
} __attribute__((aligned(64)));

typedef struct {
    hashmap_leaf **start;
    u64 len;
    u64 (*hash_function)(u64);
    bool8 (*equal_function)(u64, u64);
} hashmap;

typedef enum {
    HASHMAP_TYPE_STR,
    HASHMAP_TYPE_NUMBER,
} hashmap_types;

u64 hashmap_insert(hashmap *hashmap, u64 key, u64 data);
u64 hashmap_remove(hashmap *hashmap, u64 key);
int hashmap_lookup(hashmap *hashmap, u64 key, u64 *data);
void hashmap_create(hashmap_types type, hashmap *init_hashmap);
bool8 hashmap_contains(hashmap *hashmap, u64 key);

#endif