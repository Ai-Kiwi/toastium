#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#define PANIC(message, extra_value_1, extra_value_2, extra_value_3) kernel_panic(__FILE__ ,__LINE__ , __FUNCTION__, message, extra_value_1, extra_value_2, extra_value_3);

void kernel_panic(const char *file, const long file_line, const char *function, const char *message, long extra_value_1, long extra_value_2, long extra_value_3);

#endif