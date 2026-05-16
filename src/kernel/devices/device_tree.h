#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

#include "include/types.h"

typedef struct {
    u8 **parent_nodes; //backwards, first item is closest parents
    int node_depth;
    u8 *name;
    u8 *value;
    u32 value_len;
} device_info;

typedef struct {
    u32 size;
    u8 *end_location;
} device_info_dump_response;

void kernel_device_tree_init(u8 *kernel_end);
device_info *kernel_device_tree_pointer();
u32 kernel_device_tree_length();
u8 *kernel_device_tree_end_pointer();

#endif