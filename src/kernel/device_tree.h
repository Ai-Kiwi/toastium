#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

typedef struct {
    char *name;
    char *value;
    unsigned int value_len;
} device_info;

typedef struct {
    char *name;
    device_info *devices;
    device_node *nodes;
    unsigned device_count;
    unsigned node_count;
} device_node;

void kernel_device_tree_init(device_node *root_device_tree);

#endif