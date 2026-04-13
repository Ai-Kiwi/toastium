#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "arch_device_tree/dtb.h"
#include "kernel/devices/device_tree.h"

typedef struct {
    device_info *device_list;
    unsigned int device_list_len;
} arch_dtb_data;

void parse_dtb(char *device_tree_blob);

#endif