#include "kernel/memory/pager.h"
#include "def.h"
#include "tests/utils.h"

void test_quick_pager() {
    test_print_stage_start("Pager");

    // create and make sure are sequental.
    u64 first_addr = pg_alloc();

    for (u64 i = 1; i < 10; i++) {
        u64 next_location = pg_alloc();
        assert_u64(next_location, first_addr + (i * KERNEL_PAGE_SIZE));
    }

    // remove certain items
    pg_free(first_addr + (8 * KERNEL_PAGE_SIZE));
    pg_free(first_addr + (3 * KERNEL_PAGE_SIZE));
    pg_free(first_addr + (5 * KERNEL_PAGE_SIZE));

    // make sure added back in that order
    u64 next_loc = pg_alloc();
    assert_u64(next_loc, first_addr + (3 * KERNEL_PAGE_SIZE));
    u64 next_loc_2 = pg_alloc();
    assert_u64(next_loc_2, first_addr + (5 * KERNEL_PAGE_SIZE));
    u64 next_loc_3 = pg_alloc();
    assert_u64(next_loc_3, first_addr + (8 * KERNEL_PAGE_SIZE));

    // free pages
    for (u64 i = 0; i < 10; i++) {
        pg_free(first_addr + (i * KERNEL_PAGE_SIZE));
    }

    test_print_stage_pass();
}