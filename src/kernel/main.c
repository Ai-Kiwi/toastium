#include "../drivers/uart/uart.h"
#include "irq.h"
#include "kernel_irq.h"
#include "trap_handler.h"
#include "panic.h"
#include "stack.h"
#include "safety.h"
#include "process.h"
#include "scheduler.h"
#include "timer.h"

void kernel_main() {    
    uart_print_chars("Kernel Starting\n");

    kernel_stack_init();

    kernel_safety_test();

    
    
    init_processes();
    kernel_schedular_init();
    //uart enable irq
    uart_init();
    irq_enable(KIRQ_UART);
    irq_enable(KIRQ_TIMER);
    irq_enable(KIRQ_SOFTWARE);
    irq_enable(KIRQ_EXTERNAL);
    
    //enable irq and general interrupts
    irq_init();
    
    kernel_safety_test();

    uart_print_chars("Finished init, now running kernel\n");

    //makes timer to kick start the os
    kernel_set_timer_future_ms(5);

    while(1) {}
}

//will have a static array for each update, e.g disk and uart. Drivers can add to this then from there kernel will loop over it and do whatever with it. This way drivers never talk to services and what not.
//Will need to remember to make system where some like uart support looking while some others will result in kernel panic, depending on what in question has got to much happened




//todo
//interrupt handler
//virtual memory
//page file for under stack so system can crash when data is written below allowed stack
//program schedular
//syscall
//framebuffer
//vertio disk handler
//fat32 disk reader
//make buffers for updates auto resize instead of fixed size
//program a console system (will likely be socket and caster)
//program core utils for os