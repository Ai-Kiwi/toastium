#include "uart_16550.h"
#include "board.h"

#define UART_BASE_LOCATION 0x10000000
#define UART_STATUS_OFFSET 5
#define UART_INTERRUPT_TOGGLE_OFFSET 1
#define UART_STATUS_READY_SEND (1 << 5)
#define UART_STATUS_READY_RECEIVE (1 << 0)

void uart_16550_output_char(const char output_char) {
    volatile uart_reg_t* uart = (volatile uart_reg_t*)UART_BASE_LOCATION;
    volatile uart_reg_t* uart_status = &uart[5];

    
    //for (volatile int i=0; i<1000000; i++) {} 

    while (!(*uart_status & UART_STATUS_READY_SEND)) {}

    *uart = output_char;
}

int uart_16550_try_fetch_char() {
    //volatile unsigned int* uart = (volatile int*)UART_VALUE;
    //volatile unsigned int* uart_status = (volatile int*)UART_STATUS;

    //if (*uart_status & UART_STATUS_READY_RECEIVE) {
    //    return *uart;
    //}else{
    //    return -1;
    //}
}

void uart_16550_init() {
    volatile uart_reg_t *base_location = (volatile uart_reg_t*)UART_BASE_LOCATION;
    base_location[UART_INTERRUPT_TOGGLE_OFFSET] = 0;
    //base_location[UART_INTERRUPT_TOGGLE_OFFSET] |= 0x1;

    volatile uart_reg_t* uart = (volatile uart_reg_t*)UART_BASE_LOCATION;
    volatile uart_reg_t* uart_status = &uart[5];

    //while (!(*uart_status & UART_STATUS_READY_SEND)) {}
}