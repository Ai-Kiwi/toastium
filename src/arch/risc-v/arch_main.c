#include "drivers/uart/uart.h"
#include "arch_device_tree/dtb.h"
#include "kernel/main.h"



void arch_main(long hart_id, char *device_tree_blob) {
    uart_print_chars("Getting ready for kernel start\n");

    parse_dtb(device_tree_blob);
    //kernel_device_tree_init(dtb_data.device_list, dtb_data.device_list_len);

    kernel_main();
    
}

