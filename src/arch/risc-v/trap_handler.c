#include "../../drivers/uart/uart.h"
#include "panic.h"
#include "arch_irq.h"

void arch_trap_handler() {//will have pointer input here that points to reg data on stack
    uart_print_chars("Trap has just happened\n"); // bad idea but just here for time being while debugging
    panic("UNHANDLED_TRAP_OCCURRED",0);
}