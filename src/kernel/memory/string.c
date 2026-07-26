#include "def.h"
#include "include/types.h"

void memcpy(u64 dst, u64 src, u64 size) {
    if (ROUND_MOD_DOWN(dst, 8) != dst || ROUND_MOD_DOWN(src, 8) != src || ROUND_MOD_DOWN(size, 8) != size) {
        //not both 8 byte aligned, use byte by byte
        u8 *src_ptr = (u8 *)src;
        u8 *dst_ptr = (u8 *)dst;
        for (u64 i=0; i<size; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    }else{
        u64 *src_ptr = (u64 *)src;
        u64 *dst_ptr = (u64 *)dst;
        for (u64 i=0; i<size/8; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    }
}