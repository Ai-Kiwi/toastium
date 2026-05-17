#include "include/types.h"
#include "kernel/memory/allocator.h"
#include "drivers/uart/uart.h"

u64 kernel_radix_get_child(u64 address, u64 key, u8 depth, u8 level_depth) {
    u64 *address_table = (u64 *)address;
    u64 mask = (1UL << level_depth) - 1;
    for (s32 i = 1; i < depth+1; i++) {
        u64 level_postion = (key >> ((depth-i) * level_depth)) & mask;

        address_table = (u64 *)address_table[level_postion];
        if (address_table == 0) {
            return 0;
        }
    }
    return (u64)address_table;
}

u64 kernel_radix_create_child(u64 address, u64 key, u64 child_address, u8 depth, u8 level_depth) {
    //uart_print_u64_hex(key);
    //uart_print_str(" : ");
    u64 mask = (1UL << level_depth) - 1;
    u64 size = (1UL << level_depth);
    u64 *address_table = (u64 *)address;
    for (s32 i = 1; i < depth; i++) {
        u64 index = (key >> ((depth-i) * level_depth)) & mask;
        //uart_print_str(" -> ");
        //uart_print_u64_hex((u64)address_table);
        if (!address_table[index]) {
            u64 node_size = size * 8;
            //uart_print_str(" : (new) ");
            //uart_print_u64(node_size);

            u64 *new_node = (u64 *)kernel_allocator_acquire(node_size);
            for (s32 j = 0; j< node_size/8; j++) {
                new_node[j] = 0;
            }
            address_table[index] = (u64)new_node;
            //uart_print_str(" : ");
        }

        address_table = (u64 *)address_table[index];
    }
    u64 index = key & mask;
    u64 old_address = address_table[index];

    //uart_print_str(" -> ");
    //uart_print_u64_hex((u64)address_table);

    //uart_print_str(" >>> ");
    //uart_println_u64_hex((u64)child_address);

    address_table[index] = child_address;

    return old_address;
}

bool8 kernel_radix_remove_child(u64 address, u64 key, u8 depth, u8 level_depth) {
    u64 mask = (1UL << level_depth) - 1;
    u64 size = (1UL << level_depth);
    u64 *address_table = (u64 *)address;
    for (s32 i = 1; i < depth; i++) {
        u64 index = (key >> ((depth-i) * level_depth)) & mask;

        if (!address_table[index]) {
            return 0;
        }

        address_table = (u64 *)address_table[index];
    }
    u64 index = key & mask;
    u64 old_address = address_table[index];

    address_table[index] = 0;

    return old_address;
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