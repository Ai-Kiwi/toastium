#include "uart/uart.h"
#include "panic.h"
#include "types.h"
#include "device_tree.h"
#include "arch_dtb.h"

unsigned int read_int(char *pointer) {
    return (pointer[0] << 24) | (pointer[1] << 16) | (pointer[2] << 8) | pointer[3];
}

typedef struct {
    unsigned int magic_header;
    unsigned int total_size;
    unsigned int struct_offset;
    unsigned int strings_offset;
    unsigned int memory_offset;
    unsigned int version;
    unsigned int comptaible_version;
    unsigned int boot_cpu;
    unsigned int strings_size;
    unsigned int struct_size;
} dtb_header;




void parse_dtb(char *device_tree_blob) {
    uart_print_chars("Parsing DTB\n");

    const bool8 log_dtb = FALSE;

    dtb_header header;
    header = *(volatile dtb_header*)&device_tree_blob;

    header.magic_header = read_int(device_tree_blob);
    header.total_size = read_int(&device_tree_blob[4]);
    header.struct_offset = read_int(&device_tree_blob[8]);
    header.strings_offset = read_int(&device_tree_blob[12]);
    header.memory_offset = read_int(&device_tree_blob[16]);
    header.version = read_int(&device_tree_blob[20]);
    header.comptaible_version = read_int(&device_tree_blob[24]);
    header.boot_cpu = read_int(&device_tree_blob[28]);
    header.strings_size = read_int(&device_tree_blob[32]);
    header.struct_size = read_int(&device_tree_blob[36]);

    unsigned int magic_code = read_int(device_tree_blob);
    if (header.magic_header != 0xD00DFEED) {
        PANIC("INCORRECT_DTB_MAGIC_HEADER",header.comptaible_version);
    }
    if (header.comptaible_version != 16) {
        PANIC("INCORRECT_DTB_VERSION",header.comptaible_version);
    }

    static device_info device_list[0];
    unsigned int device_list_len = 0;

    char *node_stack[10];
    int node_stack_depth = 0;

    for (unsigned int byte_location=header.struct_offset; byte_location<header.struct_offset+header.struct_size; byte_location=byte_location+4) {
        unsigned int item_value = read_int(&device_tree_blob[byte_location]);

        switch (item_value){
        case 0x00000001: //node start
            node_stack[node_stack_depth] = (char *)byte_location;
            node_stack_depth += 1;
            if (node_stack_depth > 9) {PANIC("DTB_STACK_DEPTH_TO_HIGH", byte_location);}
            //read name
            int offset = 0;
            while (device_tree_blob[byte_location + offset + 4] != '\0'){
                offset += 1;
            }
            offset = ((offset+4) / 4) * 4;
            byte_location = byte_location + offset;
            break;
        case 0x00000002: //node end
            node_stack_depth -= 1;
            if (node_stack_depth < 0) {PANIC("DTB_STACK_DEPTH_LESS_ZERO", byte_location);}
            break;
        case 0x00000004: //no operation
            continue;
        case 0x00000003: //property
            unsigned int prop_size = read_int(&device_tree_blob[(byte_location + 4)]);
            unsigned int name_offset = read_int(&device_tree_blob[(byte_location + 8)]);

            //get name
            char *prop_name = &device_tree_blob[header.strings_offset + name_offset];

            //read the value data
            const int padded_len = ((prop_size + 3) / 4) * 4;
            byte_location += padded_len + 8;

            //add to list
            {
                device_info device;
                device.node_depth = node_stack_depth;
                device.name = prop_name;
                device.value = &device_tree_blob[(byte_location + 12)];
                device.value_len = prop_size;
                char *parent_tree[node_stack_depth]; 
                for (int i=0; i<node_stack_depth; i++) {
                    parent_tree[i] = node_stack[node_stack_depth - 1 - i];
                }
                device.parent_nodes = *parent_tree;
                //TODO: code to pass approach
                //This approach is n(O^2), which is not great. 
                //However for now it is staying, as cleanest alterative is likely 2 pass approach or some form of storing each following items as chained references. 
                //Both add more code complexity for not much performance gain as DTB is pretty small.
                device_info new_device_list[device_list_len+1];
                for (int i=0; i<device_list_len; i++) {
                    new_device_list[i] = device_list[i];
                }
                device_list_len +=1;
                new_device_list[device_list_len] = device;
            }

            break;
        case 0x00000009:
            //end code
            break;
        default:
            PANIC("DTB_UNEXPECTED_CODE", item_value);
        }
    }

    //lacking the heap allocator which is needed ot actually append this data.
    arch_dtb_data response;
    response.device_list = device_list;
    response.device_list_len = device_list_len;
    return response;

}

