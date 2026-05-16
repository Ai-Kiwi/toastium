#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "arch_device_tree/dtb.h"
#include "kernel/devices/device_tree.h"

void set_device_tree_block_location(u8 *new_device_tree_blob);
device_info_dump_response arch_parse_dtb_ram(u8 *output_location);
u32 arch_dtb_read_int(u8 *pointer);


#endif