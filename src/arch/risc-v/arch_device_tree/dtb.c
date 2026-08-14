#include "drivers/uart/uart.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/devices/device_tree.h"
#include "arch_device_tree/dtb.h"
#include "dtb.h"

u32 dtb_read_int(u8 *ptr) {
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
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


u8 *dtb = 0;

void dtb_set_dst(u8 *new_dtb) {
    dtb = new_dtb;
}

void dtb_dump(u8 *output_location, device_info_dump_response *response) {
    uart_println_str("Parsing DTB");

    if ((u64)&dtb == 0x0) {
        PANIC("BLANK_DTB_LOCATION",(u64)dtb, 0, 0);
    }

    dtb_header header;
    header = *(volatile dtb_header*)dtb;

    header.magic_header = dtb_read_int(dtb);
    header.total_size = dtb_read_int(&dtb[4]);
    header.struct_offset = dtb_read_int(&dtb[8]);
    header.strings_offset = dtb_read_int(&dtb[12]);
    header.memory_offset = dtb_read_int(&dtb[16]);
    header.version = dtb_read_int(&dtb[20]);
    header.comptaible_version = dtb_read_int(&dtb[24]);
    header.boot_cpu = dtb_read_int(&dtb[28]);
    header.strings_size = dtb_read_int(&dtb[32]);
    header.struct_size = dtb_read_int(&dtb[36]);

    if (header.magic_header != 0xD00DFEED) {
        PANIC("INCORRECT_DTB_MAGIC_HEADER",header.comptaible_version, 0, 0);
    }
    if (header.comptaible_version != 16) {
        PANIC("INCORRECT_DTB_VERSION",header.comptaible_version, header.version, 0);
    }


    u8 *cur_loc = output_location;
    device_info *node_stack[16];
    node_stack[0] = NULL;
    node_stack[1] = NULL;
    node_stack[2] = NULL;
    s32 node_depth = 0;

    for (u32 byte_location=header.struct_offset; byte_location<header.struct_offset+header.struct_size; byte_location=byte_location+4) {
        u32 item_value = dtb_read_int(&dtb[byte_location]);


        switch (item_value){
        case 0x00000001: //node start
            device_info *node = (device_info *)cur_loc;
            cur_loc += sizeof(device_info);
            node->name = (char *)&dtb[(byte_location + 4)];
            node->is_leaf = FALSE;
            node->first_child = NULL;
            node->next_sibling = NULL;
            if (node_stack[node_depth] != NULL) {
                node_stack[node_depth]->next_sibling = node;
            }
            if (node_depth > 0 && node_stack[node_depth-1]->first_child == NULL) {
                node_stack[node_depth-1]->first_child = node;
            }
            node_stack[node_depth] = node;
            node_depth += 1;
            if (node_depth >= 14) {PANIC("DTB_STACK_DEPTH_TO_HIGH", byte_location, 0, 0);}
            s32 offset = 0;
            while (dtb[byte_location + offset + 4] != '\0'){
                offset += 1;
            }
            offset = ((offset+4) / 4) * 4;
            byte_location = byte_location + offset;
            break;
        case 0x00000002: //node end
            node_stack[node_depth+1] = NULL; //removes deeper node. Idea being for finding sibling 
            node_depth -= 1;
            if (node_depth < 0) {PANIC("DTB_STACK_DEPTH_LESS_ZERO", byte_location, 0, 0);}
            break;
        case 0x00000004: //no operation
            continue;
        case 0x00000003: //property
            u32 prop_size = dtb_read_int(&dtb[(byte_location + 4)]);
            u32 name_offset = dtb_read_int(&dtb[(byte_location + 8)]);

            char *prop_name = &dtb[header.strings_offset + name_offset];


            device_info *device = (device_info *)cur_loc;
            cur_loc += sizeof(device_info);
            device->is_leaf = TRUE;
            device->name = prop_name;
            device->value = &dtb[(byte_location + 12)];
            device->value_len = prop_size;
            device->next_sibling = NULL;

            if (node_stack[node_depth] != NULL) {
                node_stack[node_depth]->next_sibling = device;

            }
            if (node_depth > 0 && node_stack[node_depth-1]->first_child == NULL) {
                node_stack[node_depth-1]->first_child = device;
            }

            node_stack[node_depth] = device;

            const s32 padded_len = ((prop_size + 3) / 4) * 4;
            byte_location += padded_len + 8;
            break;
        case 0x00000009:
            break;
        default:
            PANIC("DTB_UNEXPECTED_CODE", item_value, byte_location, 0);
        }
    }

    if (!output_location) {
        PANIC("INVALID_DEVICE_TREE_DUMP_LOCATION",0, 0, 0);
    }

    response->end_loc = output_location;
    response->root = node_stack[0];
}

u64 dtb_hart_cnt() {
    return 1;
}

device_info *device_tree_prefix_get_child(const device_info *device, char *prefix, u32 iter_num) {//not actually cheapest way this could be stored really
    device_info *cur_device = device->first_child;
    u32 iter_upto = 0;
    while (cur_device != NULL) {
        if (str_starts_with(cur_device->name, prefix)) {
            if (iter_num == iter_upto) {
                return cur_device;
            }
            iter_upto +=1;
        }
        cur_device = cur_device->next_sibling;
    }
    return NULL;
}