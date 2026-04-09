#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#define PANIC(message, extra_value) kernel_panic(__FILE__ ,__LINE__ , __FUNCTION__, message, extra_value);

void kernel_panic(const char *file, const long file_line, const char *function, const char *message, long extra_value);

#endif