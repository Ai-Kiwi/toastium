#include "../../drivers/uart/uart.h"

void arch_trap_handler() {//will have pointer input here that points to reg data on stack
    uart_print_chars("Trap has just happened"); // bad idea but just here for time being while debugging
}