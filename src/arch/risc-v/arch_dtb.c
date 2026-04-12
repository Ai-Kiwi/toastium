#include "uart/uart.h"
#include "panic.h"
#include "types.h"

void uart_print_char_hex(const char hex_value) {
    static const char hex_list[] = "0123456789ABCDEF";
    for (int i=1; i > -1; i=i-1) { //64/4=16
        int digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(&hex_list[digit_value]);
    }
}

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

    const char *node_stack[10];
    int node_stack_depth = 0;

    for (unsigned int byte_location=header.struct_offset; byte_location<header.struct_offset+header.struct_size; byte_location=byte_location+4) {
        unsigned int item_value = read_int(&device_tree_blob[byte_location]);

        for (int s=0; s<node_stack_depth; s++) {
            uart_print_chars("   ");
        }


        switch (item_value){
        case 0x00000001: //node start
            uart_print_chars("start node | ");
            node_stack[node_stack_depth] = (char *)byte_location;
            node_stack_depth += 1;
            if (node_stack_depth > 9) {PANIC("DTB_STACK_DEPTH_TO_HIGH", byte_location);}
            //read name
            int offset = 0;
            while (device_tree_blob[byte_location + offset + 4] != '\0'){
                uart_print_char(&device_tree_blob[byte_location + offset + 4]);
                offset += 1;
            }
            offset = ((offset+4) / 4) * 4;
            byte_location = byte_location + offset;
            uart_print_chars("\n");
            break;
        case 0x00000002: //node end
            node_stack[node_stack_depth] = 0;
            node_stack_depth -= 1;
            if (node_stack_depth < 0) {PANIC("DTB_STACK_DEPTH_LESS_ZERO", byte_location);}
            uart_print_chars("end node\n");
            break;
        case 0x00000004: //no operation
            uart_print_chars("No operation\n");
            continue;
        case 0x00000003: //property
            uart_print_chars("Property | Name : ");
            const unsigned int prop_size = read_int(&device_tree_blob[(byte_location + 4)]);
            const unsigned int name_offset = read_int(&device_tree_blob[(byte_location + 8)]);

            //get name
            const char *prop_name = &device_tree_blob[header.strings_offset + name_offset];
            uart_print_chars(prop_name);

            //read the value data
            uart_print_chars(" | VALUE : 0x");
            for (int value_offset=0; value_offset < prop_size; value_offset++) {
                uart_print_char_hex(device_tree_blob[(byte_location + 12 + value_offset)]);
            }
            const int padded_len = ((prop_size + 3) / 4) * 4;
            byte_location += padded_len + 8;
            uart_print_char("\n");
            break;
        case 0x00000009:
            uart_print_chars("END\n");
            //end code
            break;
        default:
            uart_print_chars("?");
            PANIC("DTB_UNEXPECTED_CODE", item_value);
        }
    }
    uart_print_chars("\n");
}

