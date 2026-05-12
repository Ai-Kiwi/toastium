#include "device_tree.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/safety.h"
#include "kernel/safety/panic.h"

static device_info *kernel_device_list;
static char *kernel_device_end = 0;
static unsigned int kernel_device_list_len = 0;

void kernel_device_tree_init(char *kernel_end) {
    device_info_dump_response device_dump_info = arch_parse_dtb_ram(kernel_end);
    kernel_device_list_len = device_dump_info.size;
    kernel_device_end = device_dump_info.end_location;
    kernel_safety_test();

    uart_println_str("printing out devices");

    kernel_device_list = (device_info *)kernel_end;

    //for (int i=0; i<kernel_device_list_len; i++) {
    //    uart_print_str("Property | Name : ");
    //    uart_print_str(kernel_device_list[i].name);
    //    uart_print_str(" | Path : ");
    //    for (int p=0; p<kernel_device_list[i].node_depth; p++) {
    //        uart_print_char(' / ');
    //        uart_print_str((char *)kernel_device_list[i].parent_nodes[p]);
    //    }
    //    uart_println_str("");
    //}
}

device_info *kernel_device_tree_pointer() {
    if (!kernel_device_list_len) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT",0,0,0);
    }

    return (device_info *)kernel_device_list;
}

unsigned int kernel_device_tree_length() {
    if (!kernel_device_list_len) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT",0,0,0);
    }
    return kernel_device_list_len;
}

 char *kernel_device_tree_end_pointer() {
    if (!kernel_device_list_len) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT",0,0,0);
    }
    return kernel_device_end;
}