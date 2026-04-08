//This layout if so that if there is more then 1 uart chip it would have a folder for each chip under one uart folder, then that uart folder would have a interface which translates calls to whichever chip in question is needed
#include "uart.h"
#include "16550/uart_16550.h"

void uart_print_chars(char *print_string) {
    for (char *p = print_string; *p != '\0' ; p++) {
        uart_16550_output_char(p);
    }
}

void uart_print_char(char *print_char) {
    uart_16550_output_char(print_char);
}

void uart_poll_input() {
    
}