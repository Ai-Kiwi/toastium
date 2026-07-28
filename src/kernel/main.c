#include "arch_trap/irq.h"
#include "drivers/uart/uart.h"
#include "kernel/devices/device_tree.h"
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
#include "kernel/process/context.h"

#include "tests/pager.h"
#include "tests/allocator.h"
#include "tests/radix.h"
#include "tests/hashmap.h"
#include "tests/utils.h"

extern u8 _kernel_end, _kernel_start, _kernel_idle_process;

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
    stack_init();
    kernel_safety_test();

    uart_println_str("fetching core count");
    u64 hart_count = dtb_hart_cnt();
    u64 dtb_location = ((u64)&_kernel_end) + (hart_count * HART_KERNEL_STACK_SIZE) + 8;

    //setup device tree
    uart_println_str("Initializing device tree");
    device_tree_init((u8*)dtb_location);

    uart_println_str("Initializing pager");
    pager_init();

    uart_println_str("Initializing allocator");
    allocator_init();
    test_pager();

    test_allocator();

    test_radix();

    test_hashmap();

    uart_println_str("Initializing process handler");
    processes_init(hart_count);

    uart_println_str("Initializing schedular");
    schedular_init();

    uart_println_str("Initializing virtual memory");
    vma_init();

    uart_println_str("Creating init process");
    create_init_process();

    //enable irq and general interrupts
    uart_println_str("Initializing irq");
    irq_init();
    irq_enable_type(IRQ_TIMER);
    irq_enable_type(IRQ_UART);
    irq_enable_type(IRQ_SOFTWARE);
    irq_enable_type(IRQ_EXTERNAL);

    uart_println_str("Preforming schedular bootstrap");
    context_bootstrap(0); //setup for running processes on core 0




    uart_println_str("Running final safety test");
    kernel_safety_test();

    uart_println_str("Finished initialization, now running kernel");

    tests_hang();

    //makes timer to kick start the os
    timer_set_future_ms(5);
}
