#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "types.h"

void asset_u64(u64 first_value, u64 second_value) {
    if (first_value != second_value) {
        PANIC("NOT_EQUAL", first_value, second_value, 0);
    }
}

void test_print_step(char *text, u64 cur_step, u64 finish_step, u64 step_size) {
    u64 prev_percent = ((cur_step - step_size) * 100) / finish_step;
    u64 percent = ((cur_step) * 100) / finish_step;
    if (prev_percent == percent) {
        return;
    } 

    uart_print_char('\r');
    uart_print_str(text);
    uart_print_u64(percent);
    uart_print_str("%");
}

void test_print_next() {
    uart_print_char('\n');
}

void tests_hang() {
    #ifndef TEST_MODE
        return;
    #endif

    while (TRUE) {}
}