#include "main.h"
#include "uart/uart.h"
#include "arch_dtb.h"



void arch_main(long hart_id, char *device_tree_blob) {
    uart_print_chars("Getting ready for kernel start\n");



    parse_dtb(device_tree_blob);

    kernel_main();
    
}

