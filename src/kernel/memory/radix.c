#include "include/types.h"
#include "kernel/memory/allocator.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"

u64 kernel_radix_get_child(u64 addr, u64 key, u8 depth, u8 depth_size) {
    u64 *addr_table = (u64 *)addr;
    u64 mask = (1UL << depth_size) - 1;
    for (s32 i = 1; i < depth+1; i++) {
        u64 level_postion = (key >> ((depth-i) * depth_size)) & mask;

        addr_table = (u64 *)addr_table[level_postion];
        if (addr_table == 0) {
            return 0;
        }
    }
    return (u64)addr_table;
}

u64 kernel_radix_create_child(u64 addr, u64 key, u64 child_addr, u8 depth, u8 depth_size) {
    u64 mask = (1UL << depth_size) - 1;
    u64 size = (1UL << depth_size);
    if (key >= (1UL << (depth_size * depth))) {
        PANIC("addr_KEY_LARGER_THEN_RADIX",key,depth,depth_size);
    }

    u64 *addr_table = (u64 *)addr;
    for (s32 i = 1; i < depth; i++) {
        u64 idx = (key >> ((depth-i) * depth_size)) & mask;
        if (!addr_table[idx]) {
            u64 node_size = size * 8;
            u64 *new_node = (u64 *)kernel_allocator_acquire(node_size);
            for (s32 j = 0; j< node_size/8; j++) {
                new_node[j] = 0;
            }
            addr_table[idx] = (u64)new_node;
        }

        addr_table = (u64 *)addr_table[idx];
    }
    u64 idx = key & mask;
    u64 old_addr = addr_table[idx];

    addr_table[idx] = child_addr;

    return old_addr;
}

void kernel_radix_delete(u64 addr, bool8 remove_leaves, u8 depth, u8 depth_size) {
    u64 size = (1UL << depth_size);
    u64 *addr_table = (u64 *)addr;

    if (depth > 1) {
        for (s32 i=0; i<size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                kernel_radix_delete(child_addr, remove_leaves, depth - 1, depth_size);
                kernel_allocator_release(child_addr);
            }
        }
    }else{
        if (remove_leaves == TRUE) {
            for (s32 i=0; i<size; i++) {
                u64 child_addr = (u64)addr_table[i];
                if (child_addr > 0) {
                    kernel_allocator_release(child_addr);
                }
            }
        }
    }
}

bool8 kernel_radix_remove_child(u64 addr, u64 key, u8 depth, u8 depth_size) {
    u64 mask = (1UL << depth_size) - 1;
    u64 size = (1UL << depth_size);
    u64 *addr_table = (u64 *)addr;
    for (s32 i = 1; i < depth; i++) {
        u64 idx = (key >> ((depth-i) * depth_size)) & mask;

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

u64 kernel_radix_create_tree(u8 level_depth) {
    u64 size = (1UL << level_depth);
    u64 node_size = size * 8;
    u64 *tree = (u64 *)kernel_allocator_acquire(node_size);

    for (s32 i = 0; i<node_size/8; i++) {
        tree[i] = 0;
    }

    return (u64)tree;
}



void kernel_radix_iter_children(u64 addr, u8 depth, u8 depth_size, u64 parameters, void (*function)(u64, u64)) {
    u64 size = (1UL << depth_size);
    u64 *addr_table = (u64 *)addr;

    if (depth > 1) {
        for (s32 i=0; i<size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                kernel_radix_iter_children(child_addr, depth - 1, depth_size, parameters, function);
            }
        }
    }else{
        //leaves
        for (s32 i=0; i<size; i++) {
            u64 child_addr = (u64)addr_table[i];
            if (child_addr > 0) {
                (*function)(child_addr,parameters);
            }
        }
    }
}