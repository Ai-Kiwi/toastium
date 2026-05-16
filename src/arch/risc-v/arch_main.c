#include "drivers/uart/uart.h"
#include "arch_device_tree/dtb.h"
#include "kernel/main.h"
#include "kernel/safety/panic.h"
#include "include/types.h"

extern u8 _kernel_end;

void arch_main(u64 hart_id, u64 device_tree_blob_address) {
    uart_println_str("Getting ready for kernel start");

    set_device_tree_block_location((u8 *)device_tree_blob_address);

    kernel_main();
}

