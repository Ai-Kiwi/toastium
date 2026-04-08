#include "../drivers/uart/uart.h"
#include "irq.h"
#include "kernel_irq.h"
#include "trap_handler.h"
#include "panic.h"

void kernel_main() {    
    uart_print_chars("Kernel Starting\n");

    //parse dbt

    //uart enable irq
    uart_init();
    IRQ_TYPES uart_irq = UART;
    irq_enable(uart_irq, 1);

    //enable irq and general interrupts
    irq_init();
    

    while(1) {}
}

//will have a static array for each update, e.g disk and uart. Drivers can add to this then from there kernel will loop over it and do whatever with it. This way drivers never talk to services and what not.
//Will need to remember to make system where some like uart support looking while some others will result in kernel panic, depending on what in question has got to much happened




//todo
//interrupt handler
//virtual memory
//program schedular
//syscall
//framebuffer
//vertio disk handler
//fat32 disk reader
//make buffers for updates auto resize instead of fixed size
