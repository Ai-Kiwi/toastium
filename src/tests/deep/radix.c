#include "kernel/memory/radix.h"
#include "drivers/uart/uart.h"
#include "kernel/memory/allocator.h"
#include "radix.h"
#include "tests/utils.h"

void test_radix() {

    test_print_stage_start("Radix");

    radix_tree tree;
    radix_create(&tree, 3, 9);

    uart_print_str("table made at : ");
    uart_println_u64_hex((u64)tree.addr);

    test_print_next();
    for (u64 i = 0; i < 100000; i++) {
        test_print_step("(1/3) Creating Data... ", i, 100000, 1);

        u64 *child_ptr = mem_alloc(8);
        *child_ptr = (i * i) + 68359;
        radix_insert(&tree, i, (u64)child_ptr);
    }

    test_print_next();
    for (u64 i = 0; i < 100000; i++) {
        test_print_step("(2/3) Confirming Data... ", i, 100000, 1);

        u64 *child_ptr = mem_alloc(8);
        *child_ptr = i;
        u64 *child = (u64 *)radix_get(&tree, i);
        if (!child) {
            uart_println_str("0");
        } else {
            if (*child != (i * i) + 68359) {
                uart_print_u64_hex((u64)child);
                uart_print_str(" : ");
                uart_println_u64(*child);
            }
        }
    }
    test_print_next();
    test_print_step("(3/3) Deleting Data... ", 0, 1, 1);

    radix_delete(&tree, TRUE);

    test_print_stage_pass();
}