#include "uart_16550.h"

#define UART_TX ((volatile char*)0x10000000)
#define UART_STATUS ((volatile char*)0x10000005)
#define UART_STATUS_READY_SEND 0x20

void uart_16550_output_char(char *output_char) {
    volatile char* uart = (volatile char*)UART_TX;
    volatile char* uart_status = (volatile char*)UART_STATUS;

    while (!(*uart_status & UART_STATUS_READY_SEND)) {}

    *uart = *output_char;
}