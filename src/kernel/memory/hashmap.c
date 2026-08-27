#include "kernel/memory/hashmap.h"
#include "drivers/uart/uart.h"
#include "hashmap.h"
#include "include/types.h"
#include "kernel/memory/allocator.h"
#include "kernel/safety/panic.h"

// adds anyway if already present, does not replace
// key used must remain valid if its a ptr
u64 hashmap_insert(hashmap *hashmap, u64 key, u64 data) {
    u64 hash = hashmap->hash_function(key);
    u64 index = hash % hashmap->len;

    volatile hashmap_leaf *new_leaf = (volatile hashmap_leaf *)mem_alloc(64);

    new_leaf->data = data;
    new_leaf->key = key;

    new_leaf->next_leaf = hashmap->start[index];
    hashmap->start[index] = (u64)new_leaf;
    return 0; // will return collision
}

// returns data if anything removed
u64 hashmap_remove(hashmap *hashmap, u64 key) {
    u64 hash = hashmap->hash_function(key);
    u64 index = hash % hashmap->len;

    volatile u64 *current_leaf_parent_ptr = (u64 *)((hashmap->start)[index]);
    volatile hashmap_leaf *current_leaf =
        (hashmap_leaf *)current_leaf_parent_ptr;

    while ((u64)current_leaf != 0) {
        if (hashmap->equal_function(current_leaf->key, key)) {
            *current_leaf_parent_ptr = (u64)current_leaf->next_leaf;
            mem_free((u64)current_leaf);
            return current_leaf->data;
        }

        current_leaf_parent_ptr = (u64 *)current_leaf->next_leaf;
        current_leaf = (hashmap_leaf *)current_leaf->next_leaf;
    }
    return 0;
}

// returns 0 if nothing fetched
u64 hashmap_get(hashmap *hashmap, u64 key) {
    u64 hash = hashmap->hash_function(key);
    u64 index = hash % hashmap->len;

    hashmap_leaf *current_leaf = (hashmap_leaf *)((hashmap->start)[index]);

    while ((u64)current_leaf != 0) {
        if (hashmap->equal_function(current_leaf->key, key)) {
            return current_leaf->data;
        }

        current_leaf = (hashmap_leaf *)current_leaf->next_leaf;
    }
    return 0;
}

// built in data

static u64 builtin_num_hash(u64 key) { return key * 0x100000001b3ULL; }

static bool8 builtin_num_equal(u64 first_key, u64 second_keys) {
    return first_key == second_keys;
}

static u64 builtin_word_hash(u64 key) {
    u8 *key_ptr = (u8 *)key;
    u64 hash = 0x3E4C3161C5E6354AULL;

    for (u64 i = 0; key_ptr[i] != '\0'; i++) {
        hash ^= key_ptr[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

static bool8 builtin_word_equal(u64 first_key, u64 second_key) {
    char *first_key_ptr = (char *)first_key;
    char *second_key_ptr = (char *)second_key;

    u64 i = 0;
    while (first_key_ptr[i] != '\0') {
        if (first_key_ptr[i] != second_key_ptr[i]) {
            return FALSE;
        }
        i++;
    }
    return first_key_ptr[i] == second_key_ptr[i];
}

void hashmap_create(hashmap_types type, hashmap *init_hashmap) {
    switch (type) {
    case HASHMAP_TYPE_STR:
        init_hashmap->equal_function = builtin_word_equal;
        init_hashmap->hash_function = builtin_word_hash;
        break;
    case HASHMAP_TYPE_NUMBER:
        init_hashmap->equal_function = builtin_num_equal;
        init_hashmap->hash_function = builtin_num_hash;
        break;
    default:
        PANIC("HASHMAP_INIT_INVALID_TYPE", type, 0, 0);
        break;
    }
}
