#ifndef ARCH_DTB_H
#define ARCH_DTB_H

#include "arch_device_tree/dtb.h"
#include "kernel/devices/device_tree.h"

void arch_set_dtb_location(u8 *new_dtb);
device_info_dump_response arch_parse_dtb_ram(u8 *output_location);
u32 arch_dtb_read_int(u8 *ptr);


#endif