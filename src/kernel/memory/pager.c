#include "endian.h"
#include "kernel/safety/panic.h"
#include "kernel/devices/device_tree.h"
#include "arch_device_tree/dtb.h"
#include "drivers/uart/uart.h"
#include "include/board.h"
#include "include/def.h"
#include "include/board.h"
#include "kernel/memory/string.h"
#include "types.h"
#include "pager.h"

//for pager bitmap 0 is free 1 is in use

volatile s64* pager_bitmap_list = 0;
s32 last_free_bitmap = 0;
s32 last_free_page_num = 0;

extern u8 _kernel_end, _kernel_start;

#define MAX_MEMORY_REGIONS 32



typedef struct {
    u64 start;
    u64 size;
} memory_region;

typedef struct {
    memory_region *reserved_regions;
    memory_region *free_regions;
    u32 reserved_cnt;
    u32 free_cnt;
} memory_region_list;

static void find_memory_regions(
    const device_info *device,
    memory_region_list *memory_regions
) {
    u32 mem_iter = 0;
    while (TRUE) {
        device_info *memory = device_tree_prefix_get_child(device, "memory",mem_iter);
        if (memory == NULL) {break;}

        device_info *memory_reg = device_tree_prefix_get_child(memory, "reg",0);
        if (memory_reg == NULL) {break;}

        u64 *value = (u64 *)memory_reg->value;

        memory_regions->free_regions[memory_regions->free_cnt].start = big_endian_u64_to_host(value[0]) + KERNEL_VMA_START;
        memory_regions->free_regions[memory_regions->free_cnt].size = big_endian_u64_to_host(value[1]);
        memory_regions->free_cnt++;

        mem_iter+=1;
    }

    device_info *root_reserved_memory = device_tree_prefix_get_child(device, "reserved-memory",0);
    if (root_reserved_memory == NULL) {PANIC("NO_RESERVED_MEMORY", 0x0, 0x0, 0x0)}

    mem_iter = 0;
    while (TRUE) {
        device_info *reserved_memory = device_tree_prefix_get_child(root_reserved_memory, "",mem_iter);
        if (reserved_memory == NULL) {break;}

        device_info *memory_reg = device_tree_prefix_get_child(reserved_memory, "reg",0);
        if (memory_reg == NULL) {break;}

        u64 *value = (u64 *)memory_reg->value;
        u64 *value_len = (u64 *)memory_reg->value_len;

        memory_regions->reserved_regions[memory_regions->reserved_cnt].start = big_endian_u64_to_host(value[0]) + KERNEL_VMA_START;
        memory_regions->reserved_regions[memory_regions->reserved_cnt].size = big_endian_u64_to_host(value[1]);
        memory_regions->reserved_cnt++;

        mem_iter+=1;
    }
}

static void remove_reserved_memory_regions(memory_region_list *memory_regions) {
    for (s32 i=0; i < memory_regions->reserved_cnt; i++) {
        memory_region *reserved_region = &memory_regions->reserved_regions[i];
        u64 reserved_region_end = reserved_region->start + reserved_region->size;

        for (s32 j=0; j < memory_regions->free_cnt; j++) {
            memory_region *free_region =  &memory_regions->free_regions[j];
            u64 free_region_end = free_region->start + free_region->size;

            //test overlap cases
            if (reserved_region->start <= free_region->start && reserved_region_end >= free_region_end) { //complete overlap/same size
                free_region->start = memory_regions->free_regions[memory_regions->free_cnt-1].start;
                free_region->size = memory_regions->free_regions[memory_regions->free_cnt-1].size;
                memory_regions->free_cnt--;
                j--;

            }else if (reserved_region->start <= free_region->start && reserved_region_end < free_region_end) { //touching start
                free_region->start = reserved_region_end;
                free_region->size -= reserved_region_end - free_region->start;

            }else if (reserved_region->start > free_region->start && reserved_region_end >= free_region_end) { //touching end
                free_region->size -= free_region->start - reserved_region->start;

            }else if (reserved_region->start > free_region->start && reserved_region_end < free_region_end) { //inside
                free_region->size -= free_region_end - reserved_region->start;

                memory_regions->free_regions[memory_regions->free_cnt].start = reserved_region_end;
                memory_regions->free_regions[memory_regions->free_cnt].size = free_region_end - reserved_region_end;
                memory_regions->free_cnt++;
                j--;
            }
        }
    }
}

void pager_init() {
    const device_info *device_info = device_tree_ptr();
    const s64 start_location = (s64)device_tree_end_ptr();

    u64 bitmap_location = (((s64)start_location+7)/8)*8; //round to byte boundary
    pager_bitmap_list = (volatile s64 *)bitmap_location;


    memory_region reserved_memory_locations[MAX_MEMORY_REGIONS];
    memory_region free_memory_locations[MAX_MEMORY_REGIONS];
    memory_region_list memory_locations;
    memory_locations.free_regions = free_memory_locations;
    memory_locations.reserved_regions = reserved_memory_locations;
    memory_locations.free_cnt = 0;
    memory_locations.reserved_cnt = 0;

    find_memory_regions(device_info, &memory_locations);

    //remove reserved areas if they fit inside of a region, if outside trim
    uart_println_str("Removing reserved from memory region");

    //reserve kernel space
    u64 start = (u64)_kernel_start;
    u64 end = (s64)start_location - (u64)_kernel_start
     + 8 + (MAX_MEMORY_REGIONS * 24); //reserve for page idx locations data
    memory_locations.reserved_regions[memory_locations.reserved_cnt].start = start;
    memory_locations.reserved_regions[memory_locations.reserved_cnt].size = end;
    memory_locations.reserved_cnt++;


    //TODO: recode this so it isn't O(n^2).
    //This is expensive for this however realistically ran once and small dataset means small cost.
    remove_reserved_memory_regions(&memory_locations);

    //log contents
    for (s32 i=0; i < memory_locations.free_cnt; i++) {
        memory_region *region = &memory_locations.free_regions[i];
        //move by region cnt if its where bitmap locations data is stored
        uart_print_str("memory region from 0x");
        uart_print_u64_hex(region->start);
        uart_print_str(" and size of ");
        uart_print_u64(region->size);
        uart_print_str("   -   0x");
        uart_print_u64_hex(region->start);
        uart_print_str("-0x");
        uart_println_u64_hex(region->start + region->size - 1);
    }


    if (memory_locations.free_cnt == 0) {
        PANIC("NO_MEMORY_REGIONS_FOR_PAGER",memory_locations.free_cnt,memory_locations.reserved_cnt,0);
    }

    //TODO: code an overlap system

    //get cnt
    pager_bitmap_list[0] = memory_locations.free_cnt;

    //set page data in idx locations.
    for (s32 i=0; i < memory_locations.free_cnt; i++) {
        memory_region *region = &memory_locations.free_regions[i];
        u64 page_region_start = ((region->start / KERNEL_PAGE_SIZE) + 1) * KERNEL_PAGE_SIZE; //round towards page chunks
        u64 page_cnt = region->size / KERNEL_PAGE_SIZE;
        s32 page_offset = ((page_cnt / 8) / KERNEL_PAGE_SIZE) + 2;
        page_cnt -= page_offset;

        pager_bitmap_list[1 + (i*3)] = page_cnt; //page cnt
        pager_bitmap_list[2 + (i*3)] = region->start; //bitmap location
        pager_bitmap_list[3 + (i*3)] = page_region_start; //page location

        uart_print_str("0x");
        uart_print_u64_hex(region->start);
        uart_print_str(" can store ");
        uart_print_u64(page_cnt);
        uart_println_str(" pages");

        //mark all as blank
        s64 *bitmap_ptr = (volatile s64 *)region->start;
        for (s32 j=0; j < (page_cnt+63)/64; j++) {
            bitmap_ptr[j] = 0;
        }
    }
}

u64 pg_alloc() { //will add cnt later u64 byte_cnt
    //unsigned page_cnt = (byte_cnt+4095) / KERNEL_PAGE_SIZE;

    const s64 page_list_cnt = pager_bitmap_list[0];
    for (s32 i=last_free_bitmap; i< page_list_cnt; i++){
        const s64 page_cnt = pager_bitmap_list[1 + (i*3)];
        volatile u64 *bitmap = (volatile u64 *)pager_bitmap_list[2 + (i*3)];
        u64 base_page = pager_bitmap_list[3 + (i*3)];

        s32 page_start = i==last_free_bitmap ? last_free_page_num/64 : 0;

        for (s32 j=page_start; j<page_cnt/64; j++) { //TODO: actually does skip some but messy to use, will revisit
            if (bitmap[j] != 0xFFFFFFFFFFFFFFFF) {
                s32 bit = __builtin_ctzl(~bitmap[j]);
                s32 free_page = (j * 64) + bit;
                last_free_bitmap = i;
                last_free_page_num = free_page;
                bitmap[j] |= BIT(bit); //mark used
                u64 page_location = base_page + (free_page * KERNEL_PAGE_SIZE);
                volatile u64 *page_ptr_s64 = (volatile u64*)page_location;
                //blank out table
                for (s32 k=0; k < KERNEL_PAGE_SIZE/8; k++) {
                    page_ptr_s64[k] = 0UL;
                }
                return page_location;
            }
        }
    }
    PANIC("OUT_OF_FREE_PAGES",0,0,0);
    return 0;
}

void pg_free(u64 location) {
    u8 *page_location = (u8 *)location;
    const s64 page_list_cnt = pager_bitmap_list[0];

    s64 closest_page_cnt = 0;
    u64 *closest_bitmap = 0;
    const u8 *closest_page_list = 0;
    s32 closet_page_num = 0;

    for (s32 i=0; i< page_list_cnt; i++){
        const u64 page_cnt = pager_bitmap_list[1 + (i*3)];
        u64 *bitmap = (u64 *)pager_bitmap_list[2 + (i*3)];
        const u8 *base_page = (u8 *)pager_bitmap_list[3 + (i*3)];

        if (base_page > closest_page_list && page_location >= base_page) {
            closest_page_cnt = page_cnt;
            closest_bitmap = bitmap;
            closest_page_list = base_page;
            closet_page_num = i;
        }
    }
    s64 page_num = (page_location - closest_page_list) / KERNEL_PAGE_SIZE;
    s64 bitmap_num = page_num / 64;
    s64 bit_num = page_num % 64;

    if (closest_bitmap == 0) {
        PANIC("PAGE_REMOVE_NO_OVERRIDE",(u64)page_location, closest_page_cnt, (u64)closest_page_list);
    }

    if (page_num < 0 || page_num >= closest_page_cnt) {
        PANIC("ATTEMPT_REMOVE_PAGE_OUT_OF_RANGE",page_num, closest_page_cnt, (u64)closest_page_list);
    }

    if (~closest_bitmap[bitmap_num] & BIT(bit_num)) {
        PANIC("DOUBLE_FREE_PAGE",(s64)page_location, (s64)page_num, (s64)closest_bitmap);
    }

    closest_bitmap[bitmap_num] &= ~BIT(bit_num);

    if (last_free_bitmap > closet_page_num || (last_free_page_num > page_num && last_free_bitmap == closet_page_num)) {
        last_free_bitmap = closet_page_num;
        last_free_page_num = page_num;
    }

}
