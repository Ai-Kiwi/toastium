#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

typedef struct {
    char *parent_nodes; //backwards, first item is closest parents
    char node_depth;
    char *name;
    char *value;
    unsigned int value_len;
} device_info;

void kernel_device_tree_init(const device_info *device_list, unsigned int device_list_len);

#endif