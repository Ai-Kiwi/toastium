#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

#include "include/types.h"

typedef struct {
    void *first_child;
    void *next_sibling;
    char *name;
    bool8 is_leaf;
    u8 *value;
    u64 value_len;
} device_info;

typedef struct {
    device_info *root;
    u8 *end_loc;
} device_info_dump_response;

void device_tree_init(u8 *kernel_end);
device_info *device_tree_ptr();
u8 *device_tree_end_ptr();

#endif