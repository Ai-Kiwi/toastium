#include "uart_16550.h"

#define UART_VALUE ((volatile char*)0x10000000)
#define UART_STATUS ((volatile char*)0x10000005)
#define UART_STATUS_READY_SEND (1 << 5)
#define UART_STATUS_READY_RECEIVE (1 << 0)
#define UART_INTERRUPT_TOGGLE ((volatile char*)0x10000001)

void uart_16550_output_char(const char *output_char) {
    volatile char* uart = (volatile char*)UART_VALUE;
    volatile char* uart_status = (volatile char*)UART_STATUS;

    while (!(*uart_status & UART_STATUS_READY_SEND)) {}

    *uart = *output_char;
}

int uart_16550_try_fetch_char() {
    volatile char* uart = (volatile char*)UART_VALUE;
    volatile char* uart_status = (volatile char*)UART_STATUS;

    if (*uart_status & UART_STATUS_READY_RECEIVE) {
        return *uart;
    }else{
        return -1;
    }
}

void uart_16550_init() {
    *(volatile char*)(UART_INTERRUPT_TOGGLE) |= 0x1;
}