#include "kernel/memory/allocator.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"
#include "tests/utils.h"
#include "types.h"

void test_allocator() {
    #ifndef TEST_MODE
        return;
    #endif

    uart_println_str("#TEST# - Testing Allocator");

    u64 value_cnt = 2000;//100000;

    u64 blank_assigns = 200000;

    u64 different_sizes = 10000;

    u64 *alloc_locations[value_cnt];

    u64 old_location = (u64)mem_alloc(32); //does leave a 32bit unfreed
    for (u64 i=0; i<(value_cnt/2); i++) {
        test_print_step("(1/8) Creating data 1st half... ", i*2, value_cnt, 2);

        u64 location = (u64)mem_alloc(32);
        u64 location_change = location - old_location;
        if (location_change > 9000) {
            PANIC("ALLOCATOR_CHANGE_LARGER_THEN_PAGE", location_change, i, 0)
        }
        old_location = location;
        alloc_locations[i] = (u64 *)location;
        *alloc_locations[i] = (i*653)+3;
    }
    //need to test removing random items and make sure they are added back the same

    kernel_safety_test();

    old_location = (u64)mem_alloc(32);
    test_print_next();
    for (u64 i=0; i<blank_assigns; i++) { //create blank must be done after as else the follow index and data page will be size jump
        test_print_step("(2/8) Creating blank, testing distance... ", i, blank_assigns, 1);

        u64 location = mem_alloc(32);
        u64 location_change = location - old_location;
        if (location_change > 9000) { //little over 8kb. Means that it has room for idx and page data change, also supports extra from start headers.
            PANIC("ALLOCATOR_CHANGE_LARGER_THEN_PAGE", location_change, i, 0)
        }
        old_location = location;
    }

    test_print_next();
    for (u64 i=0; i<different_sizes; i++) {
        test_print_step("(3/8) Creating blank, all sizes... ", i, different_sizes, 1);

        for (u64 j=0; j<12; j++) {
            u64 size = BIT(j);
            mem_alloc(size);
        }
    }

    test_print_next();
    for (u64 i=0; i<(blank_assigns/10); i++) { //create blank must be done after as else the follow index and data page will be size jump
        test_print_step("(4/8) Creating blank... ", i, blank_assigns/10, 1);

        mem_alloc(32);
    }

    test_print_next();
    old_location = (u64)mem_alloc(32);
    for (u64 i=(value_cnt/2); i<(value_cnt); i++) {
        test_print_step("(5/8) Creating data 2nd half... ", (i-(value_cnt/2))*2, value_cnt, 2);

        u64 location = (u64)mem_alloc(32);
        //uart_println_u64_hex(location);
        u64 location_change = location - old_location;
        if (location_change > 9000) { //little over 8kb. Means that it has room for idx and page data change, also supports extra from start headers.
            PANIC("ALLOCATOR_CHANGE_LARGER_THEN_PAGE", location_change, i, 0)
        }
        old_location = location;
        alloc_locations[i] = (u64 *)location;
        *alloc_locations[i] = (i*653)+3;
    }

    test_print_next();
    test_print_step("(6/8) Random freeing and setting... ", 1, 1, 1);
    //randomly sets values and frees them. On next assign these same values should be set.
    //also worth noting the high value then low value. This is designed to test the cache system
    mem_free((u64)alloc_locations[267]);
    mem_free((u64)alloc_locations[965]);
    mem_free((u64)alloc_locations[1233]);
    mem_free((u64)alloc_locations[385]);
    mem_free((u64)alloc_locations[1367]);


    *(alloc_locations[267]) = 0;
    *(alloc_locations[965]) = 0;
    *(alloc_locations[1233]) = 0;
    *(alloc_locations[385]) = 0;
    *(alloc_locations[1367]) = 0;

    u64 *value_1 = (u64 *)mem_alloc(32);
    *value_1 = (267*653)+3;
    u64 *value_2 = (u64 *)mem_alloc(32);
    *value_2 = (385*653)+3;
    u64 *value_3 = (u64 *)mem_alloc(32);
    *value_3 = (965*653)+3;
    u64 *value_4 = (u64 *)mem_alloc(32);
    *value_4 = (1233*653)+3;
    u64 *value_5 = (u64 *)mem_alloc(32);
    *value_5 = (1367*653)+3;

    test_print_next();
    for (u64 i=0; i<value_cnt; i++) {
        test_print_step("(7/8) Verifying values... ", i, value_cnt, 1);
        asset_u64(*(alloc_locations[i]), (i*653)+3);
    }

    test_print_next();
    for (u64 i=0; i<value_cnt; i++) {
        test_print_step("(8/8) Releasing some of values... ", i, value_cnt, 1);

        mem_free((u64)alloc_locations[i]);
    }
    kernel_safety_test();

    uart_println_str("\nPASS");
}