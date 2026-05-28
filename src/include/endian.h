#ifndef INCLUDE_ENDIAN_H
#define INCLUDE_ENDIAN_H

#include "include/types.h"

u64 big_endian_u64_to_host(u64 big_endian);
u64 little_endian_u64_to_host(u64 little_endian);
u32 big_endian_u32_to_host(u32 big_endian);
u32 little_endian_u32_to_host(u32 little_endian);
u16 big_endian_u16_to_host(u16 big_endian);
u16 little_endian_u16_to_host(u16 little_endian);
void init_endian_conversion();

#endif