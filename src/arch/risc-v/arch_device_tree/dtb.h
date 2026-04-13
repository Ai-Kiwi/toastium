#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "arch_device_tree/dtb.h"
#include "kernel/devices/device_tree.h"

void set_device_tree_block_location(char *new_device_tree_blob);
unsigned int arch_parse_dtb_ram(char *output_location);

#endif