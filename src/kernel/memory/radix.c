#include "kernel/memory/radix.h"
#include "drivers/uart/uart.h"
#include "include/types.h"
#include "kernel/memory/allocator.h"
#include "kernel/safety/panic.h"
#include "radix.h"

u64 radix_get(radix_tree *tree, u64 key) {
    u64 *addr_table = tree->addr;
    u64 mask = (1UL << tree->level_size) - 1;
    for (s32 i = 1; i < tree->levels + 1; i++) {
        u64 level_postion =
            (key >> ((tree->levels - i) * tree->level_size)) & mask;

        addr_table = (u64 *)addr_table[level_postion];
        if (addr_table == 0) {
            return 0;
        }
    }
    return (u64)addr_table;
}

u64 radix_insert(radix_tree *tree, u64 key, u64 value) {
    u64 mask = (1UL << tree->level_size) - 1;
    u64 size = (1UL << tree->level_size);
    if (key >= (1UL << (tree->level_size * tree->levels))) {
        PANIC("addr_KEY_LARGER_THEN_RADIX", key, tree->levels,
              tree->level_size);
    }

    u64 *addr_table = tree->addr;
    for (s32 i = 1; i < tree->levels; i++) {
        u64 idx = (key >> ((tree->levels - i) * tree->level_size)) & mask;
        if (!addr_table[idx]) {
            u64 node_size = size * 8;
            u64 *new_node = mem_alloc(node_size);
            for (s32 j = 0; j < node_size / 8; j++) {
                new_node[j] = 0;
            }
            addr_table[idx] = (u64)new_node;
        }

        addr_table = (u64 *)addr_table[idx];
    }
    u64 idx = key & mask;
    u64 old_addr = addr_table[idx];

    addr_table[idx] = value;

    return old_addr;
}

void radix_delete(radix_tree *tree, bool8 remove_leaves) {
    u64 size = (1UL << tree->level_size);
    u64 *addr_table = tree->addr;

    if (tree->levels > 1) {
        for (s32 i = 0; i < size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                radix_tree child_tree = {
                    .addr = (u64 *)child_addr,
                    .levels = tree->levels - 1,
                    .level_size = tree->level_size,
                };
                radix_delete(&child_tree, remove_leaves);
                mem_free((void *)child_addr);
            }
        }
    } else {
        if (remove_leaves == TRUE) {
            for (s32 i = 0; i < size; i++) {
                u64 child_addr = (u64)addr_table[i];
                if (child_addr > 0) {
                    mem_free((void *)child_addr);
                }
            }
        }
    }
}

bool8 radix_remove(radix_tree *tree, u64 key) {
    u64 mask = (1UL << tree->level_size) - 1;
    u64 *addr_table = tree->addr;
    for (s32 i = 1; i < tree->levels; i++) {
        u64 idx = (key >> ((tree->levels - i) * tree->level_size)) & mask;

        if (!addr_table[idx]) {
            return 0;
        }

        addr_table = (u64 *)addr_table[idx];
    }
    u64 idx = key & mask;
    u64 old_addr = addr_table[idx];

    addr_table[idx] = 0;

    return old_addr;
}

void radix_create(radix_tree *output_tree, u8 level_size, u8 levels) {
    u64 size = (1UL << level_size);
    u64 node_size = size * 8;
    u64 *tree = mem_alloc(node_size);

    for (s32 i = 0; i < node_size / 8; i++) {
        tree[i] = 0;
    }

    output_tree->addr = tree;
    output_tree->level_size = level_size;
    output_tree->levels = levels;
}

void radix_iter(radix_tree *tree, u64 parameters, void (*function)(u64, u64)) {
    u64 size = (1UL << tree->level_size);
    u64 *addr_table = tree->addr;

    if (tree->levels > 1) {
        for (s32 i = 0; i < size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                radix_tree child_tree = {
                    .addr = (u64 *)child_addr,
                    .levels = tree->levels - 1,
                    .level_size = tree->level_size,
                };
                radix_iter(&child_tree, parameters, function);
            }
        }
    } else {
        // leaves
        for (s32 i = 0; i < size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                (*function)(child_addr, parameters);
            }
        }
    }
}