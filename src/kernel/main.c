#include "drivers/uart/uart.h"
#include "kernel/trap/irq.h"
#include "kernel/trap/handler.h"
#include "kernel/safety/panic.h"
#include "kernel/memory/stack.h"
#include "kernel/safety/safety.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "kernel/timer/timer.h"
#include "arch_device_tree/dtb.h"
#include "kernel/memory/pager.h"
#include "include/board.h"

extern char _kernel_end, _kernel_start;

void kernel_main() {    
    uart_init();
    uart_println_str("Initializing kernel...");

    uart_print_str("kernel loaded at : 0x");
    uart_print_ulong_hex((unsigned long)&_kernel_start + KERNEL_VMA_START);
    uart_print_str(" - 0x");
    uart_println_ulong_hex((unsigned long)&_kernel_end + KERNEL_VMA_START);


    //setup stack
    uart_println_str("Initializing stack");
    kernel_stack_init();
    kernel_safety_test();

    //setup device tree
    uart_println_str("Initializing device tree");
    kernel_device_tree_init((char*)&_kernel_end);
    
    uart_println_str("Initializing process handler");    
    init_processes();
    kernel_schedular_init();
    
    //enable irq and general interrupts
    irq_init();

    uart_println_str("Initializing pager");
    kernel_pager_init();
    
    //uart enable irq
    uart_println_str("Initializing irq");
    irq_enable(KIRQ_UART);
    irq_enable(KIRQ_TIMER);
    irq_enable(KIRQ_SOFTWARE);
    irq_enable(KIRQ_EXTERNAL);

    uart_println_str("Running final safety test");
    kernel_safety_test();

    uart_println_str("Finished initialization, now running kernel");

    irq_start();

    //makes timer to kick start the os
    kernel_set_timer_future_ms(5);
}