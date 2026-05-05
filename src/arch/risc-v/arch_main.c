#include "drivers/uart/uart.h"
#include "arch_device_tree/dtb.h"
#include "kernel/main.h"
#include "kernel/safety/panic.h"

extern char _kernel_end;

void arch_main(long hart_id, unsigned long device_tree_blob_address) {
    uart_println_str("Getting ready for kernel start");

    set_device_tree_block_location((char *)device_tree_blob_address);

    kernel_main();
}

