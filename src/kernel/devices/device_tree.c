#include "device_tree.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "endian.h"
#include "include/types.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"

static device_info *root_device_node = NULL;
static u8 *kernel_device_end = 0;

void device_tree_init(u8 *kernel_end) {
    device_info_dump_response response;
    dtb_dump(kernel_end, &response);
    kernel_device_end = response.end_loc;
    root_device_node = response.root;
    kernel_safety_test();

    uart_println_str("printing out devices");

    device_info *address_cells_device =
        device_tree_prefix_get_child(root_device_node, "#address-cells", 0);
    if (address_cells_device == NULL) {
        PANIC("FAILED_TO_GET_ADDRESS_CELL_SIZE_INFO", 0, 0, 0)
    }
    device_info *cell_size_device =
        device_tree_prefix_get_child(root_device_node, "#size-cells", 0);
    if (cell_size_device == NULL) {
        PANIC("FAILED_TO_GET_ADDRESS_CELL_SIZE_INFO", 0, 0, 0)
    }

    u32 address_cells =
        big_endian_u32_to_host(*(u32 *)address_cells_device->value);
    u32 cell_size = big_endian_u32_to_host(*(u32 *)cell_size_device->value);

    if (address_cells != 2) {
        PANIC("THE_ADDRESS_CELL_NOT_TWO", address_cells, 0x0, 0x0);
    }

    if (cell_size != 2) {
        PANIC("THE_CELL_SIZE_NOT_TWO", cell_size, 0x0, 0x0);
    }
}

device_info *device_tree_ptr() {
    if (!kernel_device_end) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT", 0, 0, 0);
    }

    return (device_info *)root_device_node;
}

u8 *device_tree_end_ptr() {
    if (!kernel_device_end) {
        PANIC("DEVICE_TREE_FETCHED_BUT_NOT_INIT", 0, 0, 0);
    }
    return kernel_device_end;
}

static void device_tree_print_node(device_info *device, u64 cur_depth) {
    for (u64 i = 0; i < cur_depth; i++) {
        uart_print_str(" | ");
    }
    if (device->is_leaf == TRUE) {
        uart_print_str("- ");
    } else {
        uart_print_str("> ");
    }
    uart_println_str(device->name);

    if (device->is_leaf == TRUE) {
        for (u64 i = 0; i < cur_depth; i++) {
            uart_print_str(" | ");
        }
        uart_print_str("  ^ len : ");
        uart_println_u64(device->value_len);

        for (u64 i = 0; i < cur_depth; i++) {
            uart_print_str(" | ");
        }
        uart_print_str("  ^ value : ");
        for (u64 i = 0; i < device->value_len; i++) {
            uart_print_u8_hex(device->value[i]);
        }
        uart_println_str("");
    }

    if (device->first_child != NULL) {
        device_tree_print_node((device_info *)device->first_child,
                               cur_depth + 1);
    }

    if (device->next_sibling != NULL) {
        device_tree_print_node((device_info *)device->next_sibling, cur_depth);
    }
}

void device_tree_print() {
    device_info *root_device = device_tree_ptr();
    device_tree_print_node(root_device_node, 0);
}