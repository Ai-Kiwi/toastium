#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "kernel/devices/device_tree.h"

void dtb_set_dst(u8 *new_dtb);
device_info_dump_response dtb_dump(u8 *output_location);
u32 dtb_read_int(u8 *ptr);

u64 dtb_hart_cnt();

#endif