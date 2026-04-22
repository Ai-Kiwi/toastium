#include "kernel/safety/panic.h"
#include "kernel/devices/device_tree.h"
#include "include/string.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "include/board.h"
#include "include/def.h"

volatile long* pager_bitmap_list = 0;
int last_free_bitmap = 0;
int last_free_page_number = 0;

#define MAX_MEMORY_REGIONS 64

void kernel_pager_init() {
    const device_info *device_info = kernel_device_tree_pointer();
    const unsigned int device_list = kernel_device_tree_length();
    const long start_location = (long)kernel_device_tree_end_pointer();
    uart_print_ulong_hex(start_location);
    uart_println_str(" ram");

    unsigned long reserved_memory_region_start[MAX_MEMORY_REGIONS];
    unsigned long reserved_memory_region_size[MAX_MEMORY_REGIONS];
    int reserved_memory_region_count = 0;

    unsigned long memory_region_start[MAX_MEMORY_REGIONS];
    unsigned long memory_region_size[MAX_MEMORY_REGIONS];
    int memory_region_count = 0;

    unsigned long bitmap_location = (((long)start_location+7)/8)*8; //round to byte boundary
    pager_bitmap_list = (volatile long *)bitmap_location;
    
    for (int i=0; i< device_list; i++) {
        if (str_starts_with(device_info[i].name, "reg")) {
            bool8 memory_reg = FALSE;
            bool8 reserved = FALSE;
            for (int n=0; n <  device_info[i].node_depth; n++) {
                if (str_starts_with(device_info[i].parent_nodes[n], "reserved-memory")) {
                    reserved = TRUE;
                    memory_reg = TRUE;
                    break;
                }

                if (str_starts_with(device_info[i].parent_nodes[n], "memory@")) {
                    //uart_print_str(device_info[i].parent_nodes[n]);
                    //uart_print_str(" - ");
                    memory_reg = TRUE;
                }
            }

            if (memory_reg) {
                int *base_value = (int *)device_info[i].value; 
                unsigned long location = arch_dtb_read_int((char *)&base_value[0]) << 32 | arch_dtb_read_int((char *)&base_value[1]);
                unsigned long size = arch_dtb_read_int((char *)&base_value[2]) << 32 | arch_dtb_read_int((char *)&base_value[3]);

                if (reserved == FALSE) {
                    //remove regions that are touching
                    for (int i=0; i<memory_region_count; i++) {
                        uart_println_str("Memory region already defined");
                        if (memory_region_start[i] == location) {
                            if (memory_region_size[i] < size) {
                                uart_println_str("Memory region size increased");
                                memory_region_size[i] = size;
                            }
                            continue;
                        } 
                    }
                }

                //if (reserved) {
                //    uart_print_str("reserved ");
                //}
                //uart_print_str("memory region from 0x");
                //uart_print_ulong_hex(location);
                //uart_print_str(" and size of ");
                //uart_println_ulong(size);

                if (reserved) {
                    reserved_memory_region_start[reserved_memory_region_count] = location;
                    reserved_memory_region_size[reserved_memory_region_count] = size;
                    reserved_memory_region_count++;
                }else{
                    memory_region_start[memory_region_count] = location;
                    memory_region_size[memory_region_count] = size;
                    memory_region_count++;
                }
            }

        }
    }
    
    //remove reserved areas if they fit inside of a region, if outside trim
    uart_println_str("Removing reserved from memory region");

    //place a fake reserved memory address in so kernel or dtb or anything hard coded doesn't get overriden
    reserved_memory_region_start[reserved_memory_region_count] = ram_start_location;
    reserved_memory_region_size[reserved_memory_region_count] = (long)start_location - ram_start_location + 8 + (MAX_MEMORY_REGIONS * 24);
    reserved_memory_region_count++;

    //TODO: remove this as was for testing
    reserved_memory_region_start[reserved_memory_region_count] = start_location + 100000000;
    reserved_memory_region_size[reserved_memory_region_count] = 100000;
    reserved_memory_region_count++;

    //TODO: reocde this so it isn't O(n^2). This is expensive for this however realsticly this is very small and ran once so unlikely to cause large slow down. 
    for (int i=0; i < reserved_memory_region_count; i++) {
        unsigned long reserved_region_start = reserved_memory_region_start[i];
        unsigned long reserved_region_end = reserved_memory_region_start[i] + reserved_memory_region_size[i]; 
        for (int j=0; j < memory_region_count; j++) {
            unsigned long region_start = memory_region_start[j];
            unsigned long region_end = memory_region_start[j] + memory_region_size[j];

            //test overlap cases
            if (reserved_region_start <= region_start && reserved_region_end >= region_end) { //complete overlap/same size
                memory_region_start[j] = memory_region_start[memory_region_count-1];
                memory_region_size[j] = memory_region_size[memory_region_count-1];
                memory_region_count--;
                j--;

            }else if (reserved_region_start <= region_start && reserved_region_end < region_end) { //touching start
                memory_region_start[j] = reserved_region_end;
                memory_region_size[j] -= reserved_region_end - region_start; 

            }else if (reserved_region_start > region_start && reserved_region_end >= region_end) { //touching end
                memory_region_size[j] -= region_end - reserved_region_start; 

            }else if (reserved_region_start > region_start && reserved_region_end < region_end) { //inside
                memory_region_size[j] -= region_end - reserved_region_start; 

                memory_region_start[memory_region_count] = reserved_region_end; 
                memory_region_size[memory_region_count] = region_end - reserved_region_end;
                memory_region_count++;
                j--;
            }
        
        }
    }

    for (int i=0; i < memory_region_count; i++) {
        //move by region count if its where bitmap locations data is stored
        uart_print_str("memory region from 0x");
        uart_print_ulong_hex(memory_region_start[i]);
        uart_print_str(" and size of ");
        uart_print_ulong(memory_region_size[i]);
        uart_print_str("   -   0x");
        uart_print_ulong_hex(memory_region_start[i]);
        uart_print_str("-0x");
        uart_println_ulong_hex(memory_region_start[i]+memory_region_size[i]-1);
    }

    if (memory_region_count < 1) {
        PANIC("NO_MEMORY_REGIONS_FOR_PAGER",memory_region_count,0,0);
    }

    //TODO: code an overlap system



    //get count
    pager_bitmap_list[0] = memory_region_count;
    
    //loop over page locations
    for (int i=0; i < memory_region_count; i++) {
        unsigned long region_start = ((memory_region_start[i] / 4096) + 1) * 4096; //round towards page chunks
        unsigned long page_count = memory_region_size[i] / 4096;
        int page_offset = ((page_count / 8) / 4096) + 2;
        page_count -= page_offset;
        
        pager_bitmap_list[1 + (i*3)] = page_count; //page count
        pager_bitmap_list[2 + (i*3)] = memory_region_start[i]; //bitmap location
        pager_bitmap_list[3 + (i*3)] = region_start; //page location

        uart_print_str("0x");
        uart_print_ulong_hex(memory_region_start[i]);
        uart_print_str(" can store ");
        uart_print_ulong(page_count);
        uart_println_str(" pages");

        //mark all as blank
        long *bitmap_pointer = (volatile long *)memory_region_start[i];
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

    closest_bitmap[bitmap_number] ^= BIT(bit_number);

    if (last_free_bitmap > closet_page_number || (last_free_page_number > page_number && last_free_bitmap == closet_page_number)) {
        last_free_bitmap = closet_page_number;
        last_free_page_number = page_number;
    }

}

//release will look at next highest array of same location, then set that bit to make it free
//after that it will then also lower the next free location if its less


//talk about pointer possibly having missing location in write up
//talk about having to code system to get next lowest bit
//talk about how bitmap ended up swapping order
