#include "panic.h"

extern char _stack_bottom_test, _stack_top_test;

#define stack_end_magic_number 0x22B3D20B07438DF4
#define stack_top_magic_number 0x58C0E68DE152E781

void kernel_stack_init() {
    *(volatile unsigned long *)&_stack_bottom_test = stack_end_magic_number;
    *(volatile unsigned long *)&_stack_top_test = stack_top_magic_number;
}



void kernel_stack_confirm_safe() {
    if (*(volatile unsigned long *)&_stack_bottom_test != stack_end_magic_number) {
        PANIC("DATA_HAS_BEEN_WRITTEN_BELOW_STACK", *(volatile unsigned long *)&_stack_bottom_test);
    }
    if (*(volatile unsigned long *)&_stack_top_test != stack_top_magic_number) {
        PANIC("DATA_HAS_BEEN_WRITTEN_ABOVE_STACK", *(volatile unsigned long *)&_stack_top_test);
    }
}
