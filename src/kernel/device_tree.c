#include "device_tree.h"

static device_node *kernel_root_device_tree;

void kernel_device_tree_init(device_node *root_device_tree) {
    kernel_root_device_tree = root_device_tree;
}