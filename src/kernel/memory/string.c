#include "string.h"
#include "def.h"
#include "include/types.h"

void memcpy(u64 dst, u64 src, u64 size) {
    if (ROUND_MOD_DOWN(dst, 8) != dst || ROUND_MOD_DOWN(src, 8) != src ||
        ROUND_MOD_DOWN(size, 8) != size) {
        // not both 8 byte aligned, use byte by byte
        u8 *src_ptr = (u8 *)src;
        u8 *dst_ptr = (u8 *)dst;
        for (u64 i = 0; i < size; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    } else {
        u64 *src_ptr = (u64 *)src;
        u64 *dst_ptr = (u64 *)dst;
        for (u64 i = 0; i < size / 8; i++) {
            dst_ptr[i] = src_ptr[i];
        }
    }
}

s32 memcmp(u64 size, const char *src_a, const char *src_b) {
    u64 i = 0;
    const u64 *u64_src_a = (u64 *)src_a;
    const u64 *u64_src_b = (u64 *)src_b;
    while (size - i >= 4) {
        if (u64_src_a[i / 4] != u64_src_b[i / 4]) {
            return 1;
        }

        i = i + 4;
    }

    while (size - i > 0) {
        if (src_a[i] != src_b[i]) {
            return 1;
        }
        i = i + 1;
    }

    return 0;
}

s32 strcmp(const char *src_a, const char *src_b) {
    u64 i = 0;
    while (TRUE) {
        if (src_a[i] == 0x0 || src_b[i] == 0x0 || src_a[i] != src_b[i]) {
            if (src_a[i] != src_b[i]) {
                if (src_a[i] > src_b[i]) {
                    return 1;
                } else {
                    return -1;
                }
            } else {
                return 0;
            }
        }
        i++;
    }
}

void strncpy(char *dest, const char *src, unsigned long size) {
    unsigned char mask = 255;
    for (unsigned long i = 0; i < size; i++) {
        unsigned char output = src[i];
        mask = (output | -output) & mask;
        dest[i] = output & mask;
    }
}

void strscpy(char *dest, const char *src, unsigned long size) {
    unsigned long i = 0;
    for (; (i < size) && (src[i] != 0x0); i++) {
        dest[i] = src[i];
    }
    if (i == size) {
        dest[size - 1] = 0x0;
    } else {
        dest[i] = 0x0;
    }
}

bool8 str_starts_with(const char *str, const char *prefix) {
    while (*prefix != '\0') {
        if (*prefix != *str) {
            return FALSE;
        }
        str++;
        prefix++;
    }
    return TRUE;
}

char *strchr(const char *str, char c) {
    for (const char *i = str; *i != 0x0; i++) {
        if (*i == c) {
            return (char *)i;
        }
    }

    return 0;
}

u64 strlen(const char *str) {
    u64 len = 0;
    for (const char *i = str; *i != 0x0; i++) {
        len++;
    }
    return len;
}