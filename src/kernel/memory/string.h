#ifndef KERNEL_STRING_H
#define KERNEL_STRING_H

#include "types.h"

void memcpy(u64 dst, u64 src, u64 size);
void strncpy(char *dest, const char *src, u64 size);
bool8 str_starts_with(const char *str, const char *prefix);
char *strchr(const char *str, char c);
void strscpy(char *dest, const char *src, unsigned long size);
s32 memcmp(u64 size, const char *src_a, const char *src_b);
// 0 means the same 1 means different
s32 strcmp(const char *src_a, const char *src_b);
u64 strlen(const char *str);

#endif