#include "uart_16550.h"
#include "board.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"

#define UART_BASE_LOCATION 0x10000000 + KERNEL_VMA_START
#define UART_STATUS_OFFSET 5
#define UART_INTERRUPT_TOGGLE_OFFSET 1
#define UART_STATUS_READY_SEND (1 << 5)
#define UART_STATUS_READY_RECEIVE (1 << 0)

//values are times 4 as on board working on it works that way
//will later be a value that is setup from dtb

u64 uart_reg_size = 0;

void uart_16550_output_u8(const u8 output_u8) {
    volatile u8* uart = (volatile u8*)UART_BASE_LOCATION;
    volatile u8* uart_status = &uart[5*uart_reg_size];


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
    //uart_reg_size = 0;
    volatile u8 *base_location = (volatile u8*)UART_BASE_LOCATION;
    base_location[UART_INTERRUPT_TOGGLE_OFFSET] = 0;
    //base_location[UART_INTERRUPT_TOGGLE_OFFSET] |= 0x1;

    volatile u8* uart = (volatile u8*)UART_BASE_LOCATION;
    for (u64 i=1; i<9; i++) {
        volatile u8* uart_stride = &uart[7*i];
        *uart_stride = 'a';
        if (*uart_stride == 'a') {
            *uart_stride = 'c';
            if (*uart_stride == 'c') {
                uart_reg_size = i;
                break;
            }
        }
    }

    if (uart_reg_size == 0) {
        //funny thing here is they won't even get this panic as uart wouldn't work
        PANIC("FAILED_TO_FIND_VALID_UART_SIZE", 0, 0, 0);
    }
}