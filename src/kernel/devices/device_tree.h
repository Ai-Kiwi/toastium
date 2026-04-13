#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

typedef struct {
    char **parent_nodes; //backwards, first item is closest parents
    int node_depth;
    char *name;
    char *value;
    unsigned int value_len;
} device_info;

void kernel_device_tree_init();

#endif