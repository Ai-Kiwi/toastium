#include "kernel/safety/panic.h"
#include "drivers/uart/uart.h"
#include "kernel/main.h"
#include "include/board.h"

#define stack_end_magic_num 0x22B3D20B07438DF4
#define stack_top_magic_num 0x58C0E68DE152E781

extern u8 _stack_top_test, _stack_bottom_test;

void stack_init() {
    *(volatile u64 *)(&_stack_bottom_test) = stack_end_magic_num;
    *(volatile u64 *)(&_stack_top_test) = stack_top_magic_num;
    uart_print_str("STACK TOP : ");
    uart_println_u64_hex((u64)&_stack_top_test);
}



void stack_confirm() {
    if (*(volatile u64 *)(&_stack_bottom_test) != stack_end_magic_num) {
        PANIC("DATA_HAS_BEEN_WRITTEN_BELOW_STACK", *(volatile u64 *)(&_stack_bottom_test), (s64)(&_stack_bottom_test), 0);
    }
    if (*(volatile u64 *)(&_stack_top_test) != stack_top_magic_num) {
        PANIC("DATA_HAS_BEEN_WRITTEN_ABOVE_STACK", *(volatile u64 *)(&_stack_top_test), (s64)(&_stack_top_test), 0);
    }
}
