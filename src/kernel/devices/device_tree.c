#include "device_tree.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "endian.h"
#include "kernel/safety/safety.h"
#include "kernel/safety/panic.h"
#include "include/types.h"

static device_info *root_device_node = NULL;
static u8 *kernel_device_end = 0;

void device_tree_init(u8 *kernel_end) {
    device_info_dump_response response;
    dtb_dump(kernel_end, &response);
    kernel_device_end = response.end_loc;
    root_device_node = response.root;
    kernel_safety_test();

    uart_println_str("printing out devices");

    //for (s32 i=0; i<kernel_device_list_len; i++) {
    //    uart_print_str("Property | Name : ");
    //    uart_print_str(kernel_device_list[i].name);
    //    uart_print_str(" | Path : ");
    //    for (s32 p=0; p<kernel_device_list[i].node_depth; p++) {
    //        uart_print_u8(' / ');
    //        uart_print_str((u8 *)kernel_device_list[i].parent_nodes[p]);
    //    }
    //    uart_println_str("");
    //}

    device_info *address_cells_device = device_tree_prefix_get_child(root_device_node, "#address-cells", 0);
    if (address_cells_device == NULL) {PANIC("FAILED_TO_GET_ADDRESS_CELL_SIZE_INFO", 0, 0, 0)}
    device_info *cell_size_device = device_tree_prefix_get_child(root_device_node, "#size-cells", 0);
    if (cell_size_device == NULL) {PANIC("FAILED_TO_GET_ADDRESS_CELL_SIZE_INFO", 0, 0, 0)}


    u32 address_cells = big_endian_u32_to_host(*(u32 *)address_cells_device->value);
    u32 cell_size = big_endian_u32_to_host(*(u32 *)cell_size_device->value);


    if (address_cells != 2) {PANIC("THE_ADDRESS_CELL_NOT_TWO", address_cells, 0x0, 0x0);}

    if (cell_size != 2) {PANIC("THE_CELL_SIZE_NOT_TWO", cell_size, 0x0, 0x0);}
}

device_info *device_tree_ptr() {
    if (!kernel_device_end) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT",0,0,0);
    }

    return (device_info *)root_device_node;
}

u8 *device_tree_end_ptr() {
    if (!kernel_device_end) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT",0,0,0);
    }
    return kernel_device_end;
}