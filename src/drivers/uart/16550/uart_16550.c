#include "uart_16550.h"
#include "board.h"

#define UART_BASE_LOCATION 0x10000000 + KERNEL_VMA_START
#define UART_STATUS_OFFSET 5
#define UART_INTERRUPT_TOGGLE_OFFSET 1
#define UART_STATUS_READY_SEND (1 << 5)
#define UART_STATUS_READY_RECEIVE (1 << 0)

void uart_16550_output_u8(const u8 output_u8) {
    volatile uart_reg_t* uart = (volatile uart_reg_t*)UART_BASE_LOCATION;
    volatile uart_reg_t* uart_status = &uart[5];


    //for (volatile s32 i=0; i<1000000; i++) {}

    while (!(*uart_status & UART_STATUS_READY_SEND)) {}

    *uart = output_u8;
}

s32 uart_16550_try_fetch_u8() {
    //volatile u32* uart = (volatile int*)UART_VALUE;
    //volatile u32* uart_status = (volatile int*)UART_STATUS;

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