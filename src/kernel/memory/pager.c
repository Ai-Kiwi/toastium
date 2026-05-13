#include "kernel/safety/panic.h"
#include "kernel/devices/device_tree.h"
#include "include/string.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "include/board.h"
#include "include/def.h"
#include "include/board.h"

volatile long* pager_bitmap_list = 0;
int last_free_bitmap = 0;
int last_free_page_number = 0;

extern char _kernel_end, _kernel_start;

#define MAX_MEMORY_REGIONS 64

void fetch_node_type(device_info *node, bool8 *memory_reg, bool8 *reserved) {
    for (int n=0; n < node->node_depth; n++) {
        char *parent_node = node->parent_nodes[n];
        if (str_starts_with(parent_node, "reserved-memory")) {
            *reserved = TRUE;
            *memory_reg = TRUE;
            break;
        }

        if (str_starts_with(parent_node, "memory@")) {
            *memory_reg = TRUE;
        }
    }
}

typedef struct {
    unsigned long start;
    unsigned long size;
} memory_region;

typedef struct {
    memory_region *reserved_regions;
    memory_region *free_regions;
    unsigned int reserved_count;
    unsigned int free_count;
} memory_region_list;

void find_memory_regions(
    const unsigned int device_list, 
    const device_info *device_info, 
    memory_region_list *memory_regions
) {
    for (int i=0; i< device_list; i++) {
        if (!str_starts_with(device_info[i].name, "reg")) {
            continue;
        }
        bool8 memory_reg = FALSE;
        bool8 reserved = FALSE;
        fetch_node_type(&device_info[i], &memory_reg, &reserved);

        if (!memory_reg) {
            continue;
        }

        int *base_value = (int *)device_info[i].value; 
        unsigned long high = arch_dtb_read_int((char *)&base_value[0]) << 32;
        unsigned long low = arch_dtb_read_int((char *)&base_value[1]);
        unsigned long location = high | low;
        location += KERNEL_VMA_START;

        high = arch_dtb_read_int((char *)&base_value[2]) << 32;
        low = arch_dtb_read_int((char *)&base_value[3]);
        unsigned long size = high | low;

        if (reserved) {
            memory_regions->reserved_regions[memory_regions->reserved_count].start = location;
            memory_regions->reserved_regions[memory_regions->reserved_count].size = size;
            memory_regions->reserved_count++;
        }else{
            memory_regions->free_regions[memory_regions->free_count].start = location;
            memory_regions->free_regions[memory_regions->free_count].size = size;
            memory_regions->free_count++;
        }
    }
}

void remove_reserved_memory_regions(memory_region_list *memory_regions) {
    for (int i=0; i < memory_regions->reserved_count; i++) {
        memory_region *reserved_region = &memory_regions->reserved_regions[i];
        unsigned long reserved_region_end = reserved_region->start + reserved_region->size;

        for (int j=0; j < memory_regions->free_count; j++) {
            memory_region *free_region =  &memory_regions->free_regions[j];
            unsigned long free_region_end = free_region->start + free_region->size;

            //test overlap cases
            if (reserved_region->start <= free_region->start && reserved_region_end >= free_region_end) { //complete overlap/same size
                free_region->start = memory_regions->free_regions[memory_regions->free_count-1].start;
                free_region->size = memory_regions->free_regions[memory_regions->free_count-1].size;
                memory_regions->free_count--;
                j--;

            }else if (reserved_region->start <= free_region->start && reserved_region_end < free_region_end) { //touching start
                free_region->start = reserved_region_end;
                free_region->size -= reserved_region_end - free_region->start;

            }else if (reserved_region->start > free_region->start && reserved_region_end >= free_region_end) { //touching end
                free_region->size -= free_region->start - reserved_region->start;

            }else if (reserved_region->start > free_region->start && reserved_region_end < free_region_end) { //inside
                free_region->size -= free_region_end - reserved_region->start;

                memory_regions->free_regions[memory_regions->free_count].start = reserved_region_end; 
                memory_regions->free_regions[memory_regions->free_count].size = free_region_end - reserved_region_end;
                memory_regions->free_count++;
                j--;
            }
        }
    }
}

void kernel_pager_init() {
    const device_info *device_info = kernel_device_tree_pointer();
    const unsigned int device_list = kernel_device_tree_length();
    const long start_location = (long)kernel_device_tree_end_pointer();

    unsigned long bitmap_location = (((long)start_location+7)/8)*8; //round to byte boundary
    pager_bitmap_list = (volatile long *)bitmap_location;


    memory_region reserved_memory_locations[MAX_MEMORY_REGIONS];
    memory_region free_memory_locations[MAX_MEMORY_REGIONS];
    memory_region_list memory_locations;
    memory_locations.free_regions = free_memory_locations;
    memory_locations.reserved_regions = reserved_memory_locations;
    memory_locations.free_count = 0;
    memory_locations.reserved_count = 0;

    find_memory_regions(device_list, device_info, &memory_locations);

    //remove reserved areas if they fit inside of a region, if outside trim
    uart_println_str("Removing reserved from memory region");

    //reserve kernel space
    unsigned long start = (unsigned long)_kernel_start;
    unsigned long end = (long)start_location - (unsigned long)_kernel_start
     + 8 + (MAX_MEMORY_REGIONS * 24); //reserve for page index locations data
    memory_locations.reserved_regions[memory_locations.reserved_count].start = start;
    memory_locations.reserved_regions[memory_locations.reserved_count].size = end;
    memory_locations.reserved_count++;


    //TODO: recode this so it isn't O(n^2). 
    //This is expensive for this however realistically ran once and small dataset means small cost. 
    remove_reserved_memory_regions(&memory_locations);

    //log contents
    for (int i=0; i < memory_locations.free_count; i++) {
        memory_region *region = &memory_locations.free_regions[i]; 
        //move by region count if its where bitmap locations data is stored
        uart_print_str("memory region from 0x");
        uart_print_ulong_hex(region->start);
        uart_print_str(" and size of ");
        uart_print_ulong(region->size);
        uart_print_str("   -   0x");
        uart_print_ulong_hex(region->start);
        uart_print_str("-0x");
        uart_println_ulong_hex(region->start + region->size - 1);
    }


    if (memory_locations.free_count < 1) {
        PANIC("NO_MEMORY_REGIONS_FOR_PAGER",memory_locations.free_count,memory_locations.reserved_count,0);
    }

    //TODO: code an overlap system

    //get count
    pager_bitmap_list[0] = memory_locations.free_count;

    //set page data in index locations.
    for (int i=0; i < memory_locations.free_count; i++) {
        memory_region *region = &memory_locations.free_regions[i]; 
        unsigned long page_region_start = ((region->start / 4096) + 1) * 4096; //round towards page chunks
        unsigned long page_count = region->size / 4096;
        int page_offset = ((page_count / 8) / 4096) + 2;
        page_count -= page_offset;

        pager_bitmap_list[1 + (i*3)] = page_count; //page count
        pager_bitmap_list[2 + (i*3)] = page_region_start; //bitmap location
        pager_bitmap_list[3 + (i*3)] = page_region_start; //page location

        uart_print_str("0x");
        uart_print_ulong_hex(region->start);
        uart_print_str(" can store ");
        uart_print_ulong(page_count);
        uart_println_str(" pages");

        //mark all as blank
        long *bitmap_pointer = (volatile long *)region->start;
        for (int j=0; j < (page_count+63)/64; j++) {
            bitmap_pointer[j] = 0;
        }
    }
}

volatile char *kernel_pager_acquire() { //will add count later unsigned long byte_count
    //unsigned page_count = (byte_count+4095) / 4096;

    const long page_list_count = pager_bitmap_list[0];
    for (int i=last_free_bitmap; i< page_list_count; i++){
        const long page_count = pager_bitmap_list[1 + (i*3)];
        unsigned long *bitmap = (unsigned long *)pager_bitmap_list[2 + (i*3)];
        char *base_page = (char *)pager_bitmap_list[3 + (i*3)];

        int page_start = i==last_free_bitmap ? last_free_page_number/64 : 0;

        for (int j=page_start; j<page_count/64; j++) { //TODO: actually does skip some but messy to use, will revisit
            if (bitmap[j] != 0xFFFFFFFFFFFFFFFF) {
                int bit = __builtin_ctzl(~bitmap[j]);
                int free_page = (j * 64) + bit;
                last_free_bitmap = i;
                last_free_page_number = free_page;
                bitmap[j] = bitmap[j] | BIT(bit); //mark used
                volatile char *page_pointer = (volatile char*)base_page + (free_page * 4096);
                volatile unsigned long *page_pointer_long = (volatile unsigned long*)page_pointer;
                //blank out table
                for (int k=0; k < 4096/8; k++) {
                    page_pointer_long[k] = 0UL;
                }
                return page_pointer;
            }
        }
    }
    PANIC("OUT_OF_FREE_PAGES",0,0,0);
}

void kernel_pager_release(char *page_location) {
    const long page_list_count = pager_bitmap_list[0];

    long closest_page_count = 0;
    unsigned long *closest_bitmap = 0;
    char *closest_page_list = 0;
    int closet_page_number = 0;

    for (int i=0; i< page_list_count; i++){
        const unsigned long page_count = pager_bitmap_list[1 + (i*3)];
        unsigned long *bitmap = (unsigned long *)pager_bitmap_list[2 + (i*3)];
        const char *base_page = (char *)pager_bitmap_list[3 + (i*3)];

        if (base_page > closest_page_list && page_location >= base_page) {
            closest_page_count = page_count;
            closest_bitmap = bitmap;
            closest_page_list = base_page;
            closet_page_number = i;
        }
    }
    long page_number = (page_location - closest_page_list) / 4096;
    long bitmap_number = page_number / 64;
    long bit_number = page_number % 64;

    if (closest_bitmap == 0) {
        PANIC("PAGE_REMOVE_NO_OVERRIDE",(unsigned long)page_location, closest_page_count, (unsigned long)closest_page_list);
    }

    if (page_number < 0 || page_number >= closest_page_count) {
        PANIC("ATTEMPT_REMOVE_PAGE_OUT_OF_RANGE",page_number, closest_page_count, (unsigned long)closest_page_list);
    }

    if (closest_bitmap[bitmap_number] & BIT(bit_number)) {
        PANIC("DOUBLE_FREE_PAGE",(long)page_location, (long)page_number, (long)closest_bitmap);
    }

    closest_bitmap[bitmap_number] |= BIT(bit_number);

    if (last_free_bitmap > closet_page_number || (last_free_page_number > page_number && last_free_bitmap == closet_page_number)) {
        last_free_bitmap = closet_page_number;
        last_free_page_number = page_number;
    }

}