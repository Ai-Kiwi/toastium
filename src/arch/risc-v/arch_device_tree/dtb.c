#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/devices/device_tree.h"
#include "arch_device_tree/dtb.h"

u32 arch_dtb_read_int(u8 *pointer) {
    return (pointer[0] << 24) | (pointer[1] << 16) | (pointer[2] << 8) | pointer[3];
}

typedef struct {
    u32 magic_header;
    u32 total_size;
    u32 struct_offset;
    u32 strings_offset;
    u32 memory_offset;
    u32 version;
    u32 comptaible_version;
    u32 boot_cpu;
    u32 strings_size;
    u32 struct_size;
} dtb_header;


u8 *device_tree_blob = 0;

void set_device_tree_block_location(u8 *new_device_tree_blob) {
    device_tree_blob = new_device_tree_blob;
}

device_info_dump_response arch_parse_dtb_ram(u8 *output_location) {
    uart_println_str("Parsing DTB");

    if (!&device_tree_blob) {
        PANIC("BLANK_DTB_LOCATION",(u64)device_tree_blob, 0, 0);
    }

    dtb_header header;
    header = *(volatile dtb_header*)device_tree_blob;

    header.magic_header = arch_dtb_read_int(device_tree_blob);
    header.total_size = arch_dtb_read_int(&device_tree_blob[4]);
    header.struct_offset = arch_dtb_read_int(&device_tree_blob[8]);
    header.strings_offset = arch_dtb_read_int(&device_tree_blob[12]);
    header.memory_offset = arch_dtb_read_int(&device_tree_blob[16]);
    header.version = arch_dtb_read_int(&device_tree_blob[20]);
    header.comptaible_version = arch_dtb_read_int(&device_tree_blob[24]);
    header.boot_cpu = arch_dtb_read_int(&device_tree_blob[28]);
    header.strings_size = arch_dtb_read_int(&device_tree_blob[32]);
    header.struct_size = arch_dtb_read_int(&device_tree_blob[36]);

    u32 magic_code = arch_dtb_read_int(device_tree_blob);
    if (header.magic_header != 0xD00DFEED) {
        PANIC("INCORRECT_DTB_MAGIC_HEADER",header.comptaible_version, 0, 0);
    }
    if (header.comptaible_version != 16) {
        PANIC("INCORRECT_DTB_VERSION",header.comptaible_version, header.version, 0);
    }

    device_info device_list[1024];
    u8 *device_parents[1024][16];
    u32 device_list_len = 0;

    u8 *node_stack[10];
    int node_stack_depth = 0;

    for (u32 byte_location=header.struct_offset; byte_location<header.struct_offset+header.struct_size; byte_location=byte_location+4) {
        u32 item_value = arch_dtb_read_int(&device_tree_blob[byte_location]);

        //uart_print_u64_hex((u64)byte_location);
        //for (int i=0; i<node_stack_depth; i++) {
        //    uart_print_str(" |");
        //}

        switch (item_value){
        case 0x00000001: //node start
            node_stack[node_stack_depth] = (u8 *)&device_tree_blob[(byte_location + 4)];
            node_stack_depth += 1;
            if (node_stack_depth > 9) {PANIC("DTB_STACK_DEPTH_TO_HIGH", byte_location, 0, 0);}
            //read name
            //uart_print_str("* ");
            //uart_println_str(&device_tree_blob[byte_location + 4]);
            int offset = 0;
            while (device_tree_blob[byte_location + offset + 4] != '\0'){
                offset += 1;
            }
            offset = ((offset+4) / 4) * 4;
            byte_location = byte_location + offset;
            break;
        case 0x00000002: //node end
            //uart_println_str("node end");
            node_stack_depth -= 1;
            if (node_stack_depth < 0) {PANIC("DTB_STACK_DEPTH_LESS_ZERO", byte_location, 0, 0);}
            break;
        case 0x00000004: //no operation
            //uart_println_str("nop");
            continue;
        case 0x00000003: //property
            u32 prop_size = arch_dtb_read_int(&device_tree_blob[(byte_location + 4)]);
            u32 name_offset = arch_dtb_read_int(&device_tree_blob[(byte_location + 8)]);

            //get name
            u8 *prop_name = &device_tree_blob[header.strings_offset + name_offset];
            //uart_print_str("-PROP: ");
            //uart_println_str(prop_name);

            //add to list
            {
                device_info device;
                device.node_depth = node_stack_depth;
                device.name = prop_name;
                device.value = &device_tree_blob[(byte_location + 12)];
                device.value_len = prop_size;
                for (int i=0; i<node_stack_depth; i++) {
                    device_parents[device_list_len][i] = (u8 *)node_stack[node_stack_depth - 1 - i];
                }
                //TODO: code to pass approach
                //This approach is n(O^2), which is not great.
                //However for now it is staying, as cleanest alterative is likely 2 pass approach or some form of storing each following items as chained references.
                //Both add more code complexity for not much performance gain as DTB is pretty small.
                device_list[device_list_len] = device;
                device_list_len +=1;

            }

            const int padded_len = ((prop_size + 3) / 4) * 4;
            byte_location += padded_len + 8;
            break;
        case 0x00000009:
            //uart_println_str("end");
            ///end code
            break;
        default:
            PANIC("DTB_UNEXPECTED_CODE", item_value, byte_location, 0);
        }
    }

    if (!output_location) {
        PANIC("INVALID_DEVICE_TREE_DUMP_LOCATION",0, 0, 0);
    }

    //lacking the heap allocator which is needed ot actually append this data.
    device_info *device_output = (device_info *)output_location;
    for (int i=0; i<device_list_len; i++) {
        device_output[i] = device_list[i];
    }
    //add parents
    u8 **parents_location = (u8 **)&device_output[device_list_len];
    int parent_offset = 0;
    for (int i=0; i<device_list_len; i++) {
        device_output[i].parent_nodes = parents_location + parent_offset;
        for (int p=0; p < device_output[i].node_depth; p++) {
            parents_location[parent_offset] = (u8 *)(device_parents[i][p]);
            parent_offset++;
        }
    }

    device_info_dump_response response;
    response.end_location = (u8 *)&parents_location[parent_offset+1];
    response.size = device_list_len;

    uart_print_str("DTB devices found : ");
    uart_println_s64((s64)device_list_len);


    return response;
    //return response;

}

