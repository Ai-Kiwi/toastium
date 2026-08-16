#include "endian.h"
#include "include/types.h"

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1

u16 endian_type;

static u64 flip_u64(u64 input) {
    return ((0x00000000000000FF & input) << (8 * 7)) |
           ((0x000000000000FF00 & input) << (8 * 5)) |
           ((0x0000000000FF0000 & input) << (8 * 3)) |
           ((0x00000000FF000000 & input) << (8 * 1)) |
           ((0x000000FF00000000 & input) >> (8 * 1)) |
           ((0x0000FF0000000000 & input) >> (8 * 3)) |
           ((0x00FF000000000000 & input) >> (8 * 5)) |
           ((0xFF00000000000000 & input) >> (8 * 7));
}

u64 big_endian_u64_to_host(u64 big_endian) {
    if (endian_type == BIG_ENDIAN) {
        return big_endian;
    }
    return flip_u64(big_endian);
}

u64 little_endian_u64_to_host(u64 little_endian) {
    if (endian_type == LITTLE_ENDIAN) {
        return little_endian;
    }
    return flip_u64(little_endian);
}

static u32 flip_u32(u32 input) {
    return ((0x000000FF & input) << (8 * 3)) |
           ((0x0000FF00 & input) << (8 * 1)) |
           ((0x00FF0000 & input) >> (8 * 1)) |
           ((0xFF000000 & input) >> (8 * 3));
}

u32 big_endian_u32_to_host(u32 big_endian) {
    if (endian_type == BIG_ENDIAN) {
        return big_endian;
    }
    return flip_u32(big_endian);
}

u32 little_endian_u32_to_host(u32 little_endian) {
    if (endian_type == LITTLE_ENDIAN) {
        return little_endian;
    }
    return flip_u32(little_endian);
}

static u16 flip_u16(u16 input) {
    return ((0x00FF & input) << (8 * 1)) | ((0xFF00 & input) >> (8 * 1));
}

u16 big_endian_u16_to_host(u16 big_endian) {
    if (endian_type == BIG_ENDIAN) {
        return big_endian;
    }
    return flip_u16(big_endian);
}

u16 little_endian_u16_to_host(u16 little_endian) {
    if (endian_type == LITTLE_ENDIAN) {
        return little_endian;
    }
    return flip_u16(little_endian);
}

void init_endian_conversion() {
    endian_type = 1;
    u8 *ptr = (u8 *)&endian_type;
    if (*ptr == 0) {
        endian_type = BIG_ENDIAN;
    } else {
        endian_type = LITTLE_ENDIAN;
    }
}