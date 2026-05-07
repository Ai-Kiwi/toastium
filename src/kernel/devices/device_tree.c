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

    //
    //unsigned int prop_size = read_int(&device_tree_blob[(byte_location + 4)]);
    //unsigned int name_offset = read_int(&device_tree_blob[(byte_location + 8)]);
    //
    ////get name
    //char *prop_name = &device_tree_blob[header.strings_offset + name_offset];
    //
    //
    ////read the value data
    //uart_print_chars(" | VALUE : 0x");
    //for (int value_offset=0; value_offset < prop_size; value_offset++) {
    //    uart_print_char_hex(device_tree_blob[(byte_location + 12 + value_offset)]);
    //}
    //const int padded_len = ((prop_size + 3) / 4) * 4;
    //byte_location += padded_len + 8;
    //uart_print_char("\n");
    //
    ////add to list
    //{
    //    device_info device;
    //    device.node_depth = node_stack_depth;
    //    device.name = prop_name;
    //    device.value = &device_tree_blob[(byte_location + 12)];
    //    device.value_len = prop_size;
    //    char *parent_tree[node_stack_depth]; 
    //    for (int i=0; i<node_stack_depth; i++) {
    //        parent_tree[i] = node_stack[node_stack_depth - 1 - i];
    //    }
    //    device.parent_nodes = *parent_tree;
    //    //TODO: code to pass approach
    //    //This approach is n(O^2), which is not great. 
    //    //However for now it is staying, as cleanest alterative is likely 2 pass approach or some form of storing each following items as chained references. 
    //    //Both add more code complexity for not much performance gain as DTB is pretty small.
    //    device_info *new_device_list[device_list_len+1];
    //    for (int i=0; i<device_list_len; i++) {
    //        new_device_list[i] = device_list[i];
    //    }
    //    device_list_len +=1;
    //    new_device_list[device_list_len] = &device;
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
f