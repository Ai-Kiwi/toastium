#ifndef DEVICE_TREE_H
#define DEVICE_TREE_H

typedef struct {
    char **parent_nodes; //backwards, first item is closest parents
    int node_depth;
    char *name;
    char *value;
    unsigned int value_len;
} device_info;

typedef struct {
    unsigned int size;
    char *end_location;
} device_info_dump_response;

void kernel_device_tree_init();
device_info *kernel_device_tree_pointer();
unsigned int kernel_device_tree_length();
char *kernel_device_tree_end_pointer();

#endif