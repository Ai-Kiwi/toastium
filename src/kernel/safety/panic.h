#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include "include/types.h"

#define PANIC(message, extra_value_1, extra_value_2, extra_value_3) kernel_panic(__FILE__ ,__LINE__ , __FUNCTION__, message, extra_value_1, extra_value_2, extra_value_3);

void kernel_panic(const u8 *file, const s64 file_line, const u8 *function, const u8 *message, s64 extra_value_1, s64 extra_value_2, s64 extra_value_3);

#endif