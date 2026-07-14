#include "kernel/memory/pager.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "tests/utils.h"
#include "kernel/safety/safety.h"

void test_pager() {
    #ifndef TEST_MODE
        return;
    #endif


    uart_println_str("#TEST# - Testing Pager");

    u64 value_cnt = 2000;//100000;

    u64 blank_assigns = 1000000;

    u64 first_loc = (u64)kernel_pager_acquire();
    u64 last_loc = first_loc;
    for (u64 i=0; i<value_cnt; i++) {
        test_print_step("(1/3) Creating pager data... ", i, value_cnt, 1);

        u64 loc = (u64)kernel_pager_acquire();
        u64 loc_change = loc - last_loc;
        if (loc_change > 4192) {
            PANIC("FOLLOWING_PAGE_TO_FAR", loc_change, i, 0)
        }
        last_loc = loc;
        if (loc % KERNEL_PAGE_SIZE != 0) {
            PANIC("KERNEL_PAGE_NOT_ALIGNED", loc, 0, 0)
        }

        u64 *location_ptr = (u64 *)loc;
        *location_ptr = (i*312)+86;
    }

    kernel_safety_test();

    test_print_next();
    test_print_step("(2/3) Releasing first page data... ", 1, 1, 1);
    kernel_pager_release(first_loc);

    test_print_next();
    for (u64 i=0; i<value_cnt ; i++) {
        u64 *loc = (u64 *)(first_loc+((i+1)*KERNEL_PAGE_SIZE));
        test_print_step("(3/3) Releasing and confirming pager data... ", i, value_cnt, 1);
        kernel_pager_release((u64)loc);
        asset_u64(*(u64 *)(loc), (i*312)+86);
    }

    kernel_safety_test();

    uart_println_str("\nPASS");
}