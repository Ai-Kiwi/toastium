#include "kernel/memory/radix.h"
#include "drivers/uart/uart.h"
#include "tests/utils.h"
#include "radix.h"

void test_radix() {
    #ifndef TEST_MODE
        return;
    #endif


    uart_println_str("#TEST# - Testing Radix");

    u64 tree_root = radix_create(4);

    uart_print_str("table made at : ");
    uart_println_u64_hex(tree_root);

    test_print_next();
    for (u64 i = 0; i<100000; i++) {
        test_print_step("(1/3) Creating Data... ", i, 100000, 1);

        u64 *child_ptr = (u64 *)mem_alloc(8);
        *child_ptr = (i*i)+68359;
        radix_insert(tree_root, i, (u64)child_ptr, 9, 3);
    }

    test_print_next();
    for (u64 i = 0; i<100000; i++) {
        test_print_step("(2/3) Confirming Data... ", i, 100000, 1);

        u64 *child_ptr = (u64 *)mem_alloc(8);
        *child_ptr = i;
        u64 *child = (u64 *)radix_get(tree_root, i, 9, 3);
        if (!child) {
            uart_println_str("0");
        }else{
            if (*child != (i*i)+68359) {
                uart_print_u64_hex((u64)child);
                uart_print_str(" : ");
                uart_println_u64(*child);
            }
        }
    }
    test_print_next();
    test_print_step("(3/3) Deleting Data... ", 0, 1, 1);

    radix_delete(tree_root, TRUE, 9, 3);

    uart_println_str("\nPASS");
}