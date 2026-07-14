#include "drivers/uart/uart.h"
#include "tests/utils.h"
#include "kernel/memory/hashmap.h"
#include "kernel/memory/pager.h"
#include "include/def.h"

void test_hashmap() {
    #ifndef TEST_MODE
        return;
    #endif

    uart_println_str("#TEST# - Testing Hashmap");

    kernel_hashmap hashmap;
    u64 *hashmap_loc = (u64 *)kernel_pager_acquire();
    hashmap.start = hashmap_loc;
    hashmap.len = KERNEL_PAGE_SIZE / 8;
    kernel_hashmap_create(KHASHMAP_TYPE_NUMBER, &hashmap);

    test_print_next();
    for (u64 i=0; i<100000; i++) {
        test_print_step("(1/2) Creating data... ", i, 100000, 1);
        kernel_hashmap_insert(&hashmap, i, (i*32)+36);
    }

    test_print_next();
    for (u64 i=0; i<100000; i++) {
        test_print_step("(2/2) Confirming data... ", i, 100000, 1);
        u64 data = kernel_hashmap_fetch(&hashmap, i);
        asset_u64((i*32)+36, data);
    }

    test_print_next();

    //create hashmap
    //assign a bunch of values
    //make sure values are all right
    //make sure none of the hashmaps get to many kids (see that it assigns right, likely a custom function of some kind)

    uart_println_str("\nPASS");
}