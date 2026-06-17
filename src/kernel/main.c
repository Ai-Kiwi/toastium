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
#include "kernel/memory/allocator.h"
#include "kernel/memory/radix.h"
#include "include/endian.h"
#include "arch_vma/virtual_memory.h"

extern u8 _kernel_end, _kernel_start;

void kernel_main() {
    uart_init();
    uart_println_str("Initializing kernel...");

    uart_print_str("kernel loaded at : 0x");
    uart_print_u64_hex((u64)&_kernel_start + KERNEL_VMA_START);
    uart_print_str(" - 0x");
    uart_println_u64_hex((u64)&_kernel_end + KERNEL_VMA_START);

    init_endian_conversion();

    //setup stack
    uart_println_str("Initializing stack");
    kernel_stack_init();
    kernel_safety_test();

    uart_println_str("fetching core count");
    u64 hart_count = arch_dtb_get_hart_count();
    u64 dtb_location = ((u64)&_kernel_end) + (hart_count * HART_KERNEL_STACK_SIZE) + 8;

    //setup device tree
    uart_println_str("Initializing device tree");
    kernel_device_tree_init((u8*)dtb_location);

    uart_println_str("Initializing pager");
    kernel_pager_init();

    uart_println_str("Initializing allocator");
    kernel_allocator_init();

    uart_println_str("Initializing process handler");
    kernel_processes_init(hart_count);
    kernel_schedular_init();

    uart_println_str("Initializing virtual memory");
    arch_vma_init();

    //enable irq and general interrupts
    irq_init();

    //uart enable irq
    uart_println_str("Initializing irq");
    irq_enable(KIRQ_UART);
    irq_enable(KIRQ_TIMER);
    irq_enable(KIRQ_SOFTWARE);
    irq_enable(KIRQ_EXTERNAL);

    uart_println_str("Running final safety test");
    kernel_safety_test();

    uart_println_str("Finished initialization, now running kernel");

    //makes timer to kick start the os
    kernel_set_timer_future_ms(5);
}
