#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "kernel/devices/device_tree.h"

void dtb_set_dst(u8 *new_dtb);
void dtb_dump(u8 *output_location, device_info_dump_response *response);
u32 dtb_read_int(u8 *ptr);
device_info *device_tree_prefix_get_child(const device_info *device, char *prefix, u32 iter_num);

u64 dtb_hart_cnt();

#endif