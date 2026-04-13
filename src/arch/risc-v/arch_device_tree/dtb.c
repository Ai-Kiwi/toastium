#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/devices/device_tree.h"
#include "arch_device_tree/dtb.h"

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


char *device_tree_blob = 0;

void set_device_tree_block_location(char *new_device_tree_blob) {
    device_tree_blob = new_device_tree_blob;
}

unsigned int arch_parse_dtb_ram(char *output_location) {
    uart_println_str("Parsing DTB");

    if (!&device_tree_blob) {
        PANIC("BLANK_DTB_LOCATION",(unsigned long)&device_tree_blob, 0, 0);
    }

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
        PANIC("INCORRECT_DTB_MAGIC_HEADER",header.comptaible_version, 0, 0);
    }
    if (header.comptaible_version != 16) {
        PANIC("INCORRECT_DTB_VERSION",header.comptaible_version, header.version, 0);
    }

    device_info device_list[1024];
    char *device_parents[1024][16];
    unsigned int device_list_len = 0;

    char *node_stack[10];
    int node_stack_depth = 0;

    for (unsigned int byte_location=header.struct_offset; byte_location<header.struct_offset+header.struct_size; byte_location=byte_location+4) {
        unsigned int item_value = read_int(&device_tree_blob[byte_location]);

        switch (item_value){
        case 0x00000001: //node start
            node_stack[node_stack_depth] = (char *)&device_tree_blob[(byte_location + 4)];
            node_stack_depth += 1;
            if (node_stack_depth > 9) {PANIC("DTB_STACK_DEPTH_TO_HIGH", byte_location, 0, 0);}
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
            if (node_stack_depth < 0) {PANIC("DTB_STACK_DEPTH_LESS_ZERO", byte_location, 0, 0);}
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
                for (int i=0; i<node_stack_depth; i++) {
                    device_parents[device_list_len][i] = (char *)node_stack[node_stack_depth - 1 - i];
                }
                //TODO: code to pass approach
                //This approach is n(O^2), which is not great. 
                //However for now it is staying, as cleanest alterative is likely 2 pass approach or some form of storing each following items as chained references. 
                //Both add more code complexity for not much performance gain as DTB is pretty small.
                device_list[device_list_len] = device;
                device_list_len +=1;
                
            }

            break;
        case 0x00000009:
            //end code
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
    char **parents_location = (char **)&device_output[device_list_len];
    int parent_offset = 0;
    for (int i=0; i<device_list_len; i++) {
        device_output[i].parent_nodes = parents_location + parent_offset;
        for (int p=0; p < device_output[i].node_depth; p++) {
            parents_location[parent_offset] = (char *)(device_parents[i][p]);
            parent_offset++;
        }
    }
        
    return device_list_len;
    //return response;

}

