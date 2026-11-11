#include "allocator.h"
#include "drivers/uart/uart.h"
#include "include/def.h"
#include "include/types.h"
#include "kernel/memory/pager.h"
#include "kernel/safety/panic.h"
#include "utils.h"
#include <stdalign.h>

// worth noting, zero is free here, one is in use. This is flipped of pager.

#define max_slots_per_index_table ((KERNEL_PAGE_SIZE - 8) / 8)

typedef struct {
    u64 *page;
    u64 cnt;
} __attribute__((aligned(64))) bump_state;

alignas(64) u64 root_idx_page[256]; // multiple of 64, so cache aligned

static u32 max_cnt_per_page(u64 size) { return (KERNEL_PAGE_SIZE - 64) / size; }

void allocator_init() {
    for (s32 i = 0; i < 256; i++) {
        root_idx_page[i] = 0;
    }
}

static void *new_data_page(u64 size) {
    // could make sure everything is zero here, pager already handles this
    u64 *data_page = pg_alloc();
    data_page[1] = size;
    return data_page;
}

void *mem_alloc(u64 size_bytes) {
    if (!size_bytes) {
        PANIC("ATTEMPT_ALLOCATE_ZERO_BYTES", 0, 0, 0);
    }
    u64 safe_size_bytes = MAX(16, size_bytes);
    u64 leading_zeros = __builtin_clzl(safe_size_bytes);
    u64 compressed_size = (63 - leading_zeros);
    u64 pow2_size = 1UL << compressed_size;
    if (pow2_size != safe_size_bytes) { // round up
        pow2_size = pow2_size << 1;
        compressed_size++;
    }
    if (pow2_size >= KERNEL_PAGE_SIZE) {
        PANIC("ATTEMPT_ALLOCATE_LARGER_THEN_PAGE", pow2_size, 0, 0);
    }

    if (!root_idx_page[compressed_size]) {
        root_idx_page[compressed_size] = (u64)new_data_page(pow2_size);
    }

    const u64 data_per_page = max_cnt_per_page(pow2_size);
    const u64 blocks_per_page = (ROUND_MOD_UP(data_per_page, 64) / 64);
    u64 *current_data_page = (u64 *)root_idx_page[compressed_size];

    // [next (8byte), size (8byte), bitmap (48 bytes)](64 bytes)
    //[all data](4032 bytes)

    while (TRUE) {

        for (u64 i = 0; i < blocks_per_page; i++) {
            u64 block = current_data_page[i + 2];
            if (block == 0xFFFFFFFFFFFFFFFF) {
                continue;
            };

            u64 bit = __builtin_ctzl(~block);
            u64 idx = (i * 64) + bit;

            if (idx >= data_per_page) {
                continue;
            }

            if ((block & BIT(bit)) != 0UL) {
                PANIC("DOUBLE_ACQUIRE_PAGE", i, pow2_size, idx);
            }

            current_data_page[i + 2] |= BIT(bit);

            return (void *)(((u64)current_data_page) + 64 + (idx * pow2_size));
        }

        if (current_data_page[0] == 0) {
            current_data_page[0] = (u64)new_data_page(pow2_size);
        }
        current_data_page = (u64 *)current_data_page[0];
    }
}

void mem_free(void *location) {
    u64 *current_data_page =
        (u64 *)(ROUND_MOD_DOWN((u64)location, KERNEL_PAGE_SIZE));

    // fetch the page location and which num it is
    const u64 size = current_data_page[1];

    u64 slot = (((u64)location & 0xFFF) - 64) / size;

    u64 idx = slot / 64;
    u64 bit = (slot % 64);

    if ((current_data_page[idx + 2] & BIT(bit)) == 0x0) {
        PANIC("ALLOCATOR_RELEASE_FREE", 0, 0, 0);
    }

    current_data_page[idx + 2] &= ~BIT(bit);
}

// TODO: Add caching so can know where to start looking instant
