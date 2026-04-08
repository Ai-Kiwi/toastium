#include "../../drivers/uart/uart.h"

void arch_trap_handler() {//will have pointer input here that points to reg data on stack
    uart_print_chars("Trap has just happened"); // bad idea but just here for time being while debugging
}

void enable_traps() {
    asm volatile("csrs sie, %0" :: "r"(1 << 11)); //enable external traps
    asm volatile("csrs sstatus, %0" :: "r"(1 << 3)); //enable global interrupts
}