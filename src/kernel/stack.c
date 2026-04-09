#include "panic.h"

extern char _stack_bottom_test;

#define stack_end_magic_number 0x22B3D20B07438DF4

void kernel_stack_init() {
    *(volatile unsigned long *)&_stack_bottom_test = stack_end_magic_number;
}



void kernel_stack_confirm_safe() {
    if (*(volatile unsigned long *)&_stack_bottom_test != stack_end_magic_number) {
        PANIC("DATA_HAS_BEEN_WRITTEN_BELOW_STACK", *(volatile unsigned int *)&_stack_bottom_test);
    }
}
