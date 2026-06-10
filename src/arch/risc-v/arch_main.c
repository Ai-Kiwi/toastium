#include "drivers/uart/uart.h"
#include "arch_device_tree/dtb.h"
#include "kernel/main.h"
#include "kernel/safety/panic.h"
#include "include/types.h"

extern u8 _kernel_end;

void arch_main(u64 hart_id, u64 dtb_addr) {
    uart_println_str("Getting ready for kernel start");

    arch_set_dtb_location((u8 *)dtb_addr);

    kernel_main();
}

