#include "kernel/memory/allocator.h"
#include "drivers/uart/uart.h"
#include "tests/utils.h"
#include "types.h"

const u64 allocator_sizes[] = {1, 3, 4, 12, 131, 124, 623, 1000, 2020};
#define ALLOCATOR_SIZES_CNT                                                    \
    (sizeof(allocator_sizes) / sizeof(allocator_sizes[0]))

static u64 allocator_sizes_start[ALLOCATOR_SIZES_CNT][5];

static u64 next_iter_value(u64 size, u64 past_value) {
    u64 value = past_value;
    value ^= size;
    value *= 0x100000001b3ULL;
    return value;
}

static u64 start_iter_value(u64 size, u64 seed) {
    u64 hash = 0x3E4C3161C5E6354AULL;
    hash = next_iter_value(size, hash);
    hash = next_iter_value(seed, hash);
    return hash;
}

static void set_size(void *start, u64 size, u64 seed) {
    u8 *ptr = start;

    u64 value = start_iter_value(size, seed);
    for (u64 i = 0; i < size; i++) {
        value = next_iter_value(size, value);
        *ptr = value % 256;
        ptr++;
    }
}

static void confirm_size(void *start, u64 size, u64 seed) {
    u8 *ptr = start;

    u64 value = start_iter_value(size, seed);
    for (u64 i = 0; i < size; i++) {
        value = next_iter_value(size, value);
        u8 expected = value % 256;
        assert_u64(expected, *ptr);
        ptr++;
    }
}

void test_quick_allocator() {
    test_print_stage_start("Allocator");

    // create values
    for (u64 i = 0; i < ALLOCATOR_SIZES_CNT; i++) {
        const u64 size = allocator_sizes[i];

        for (u64 j = 0; j < 5; j++) {
            u8 *ptr = (u8 *)mem_alloc(size);
            allocator_sizes_start[i][j] = (u64)ptr;
            set_size(ptr, size, j);
        }
    }

    // make sure values are the same
    for (u64 i = 0; i < ALLOCATOR_SIZES_CNT; i++) {
        const u64 size = allocator_sizes[i];

        for (u64 j = 0; j < 5; j++) {
            u8 *ptr = (u8 *)allocator_sizes_start[i][j];
            confirm_size(ptr, size, j);
        }
    }

    // test if freeing works
    for (u64 i = 0; i < ALLOCATOR_SIZES_CNT; i++) {
        mem_free(allocator_sizes_start[i][1]);

        mem_free(allocator_sizes_start[i][3]);

        mem_free(allocator_sizes_start[i][2]);
    }

    // make sure it returns in same order as before
    for (u64 i = 0; i < ALLOCATOR_SIZES_CNT; i++) {
        const u64 size = allocator_sizes[i];

        assert_u64(allocator_sizes_start[i][1], mem_alloc(size));
        assert_u64(allocator_sizes_start[i][2], mem_alloc(size));
        assert_u64(allocator_sizes_start[i][3], mem_alloc(size));
    }

    for (u64 i = 0; i < ALLOCATOR_SIZES_CNT; i++) {
        for (u64 j = 0; j < 5; j++) {
            mem_free(allocator_sizes_start[i][j]);
        }
    }

    test_print_stage_pass();
}