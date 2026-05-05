#include "kernel/safety/panic.h"
#include "drivers/uart/uart.h"
#include "kernel/main.h"
#include "include/board.h"

#define stack_end_magic_number 0x22B3D20B07438DF4
#define stack_top_magic_number 0x58C0E68DE152E781

extern char _stack_top_test, _stack_bottom_test;

void kernel_stack_init() {
    *(volatile unsigned long *)(&_stack_bottom_test) = stack_end_magic_number;
    *(volatile unsigned long *)(&_stack_top_test) = stack_top_magic_number;
    uart_print_str("STACK TOP : ");
    uart_println_ulong_hex((unsigned long)&_stack_top_test);
}



void kernel_stack_confirm_safe() {
    if (*(volatile unsigned long *)(&_stack_bottom_test) != stack_end_magic_number) {
        PANIC("DATA_HAS_BEEN_WRITTEN_BELOW_STACK", *(volatile unsigned long *)(&_stack_bottom_test), (long)(&_stack_bottom_test), 0);
    }
    if (*(volatile unsigned long *)(&_stack_top_test) != stack_top_magic_number) {
        PANIC("DATA_HAS_BEEN_WRITTEN_ABOVE_STACK", *(volatile unsigned long *)(&_stack_top_test), (long)(&_stack_top_test), 0);
    }
}
