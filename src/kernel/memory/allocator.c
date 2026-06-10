#include "kernel/safety/panic.h"
#include "kernel/memory/pager.h"
#include "include/types.h"
#include "include/def.h"
#include "drivers/uart/uart.h"

//worth noting, zero is free here, one is in use. This is flipped of pager.

#define max_idx_page_bitmap_entrys ((KERNEL_PAGE_SIZE - 8) / 8)

u64 root_idx_page[256];
u64 current_bump_page;
u64 current_bump_cnt;

u32 max_cnt_per_page(u64 size) {
    u64 entry_size = size + 8; //add headers
    return KERNEL_PAGE_SIZE / entry_size;
}

void kernel_allocator_init() {
    for (s32 i=0; i<256; i++) {
        root_idx_page[i] = 0;
    }

    current_bump_page = kernel_pager_acquire();
    current_bump_cnt = 0;
}

u64 new_idx_page() {
    //could make sure everything is zero here, pager already handles this
    u64 idx_page = (u64)kernel_pager_acquire();
    return idx_page;
}

u64 new_data_page() {
    //could make sure everything is zero here, pager already handles this
    u64 data_page = (u64)kernel_pager_acquire();
    return data_page;
}

u64 kernel_allocator_acquire(u64 size_bytes) {
    if (!size_bytes) {
        PANIC("ATTEMPT_ALLOCATE_ZERO_BYTES",0,0,0);
    }
    u64 leading_zeros = __builtin_clzl(size_bytes);
    u64 compressed_size = (63-leading_zeros);
    u64 pow2_size = 1UL << compressed_size;
    if (pow2_size != size_bytes) { //round up
        pow2_size = pow2_size << 1;
        compressed_size++;
    }
    if (pow2_size >= KERNEL_PAGE_SIZE) {
        PANIC("ATTEMPT_ALLOCATE_LARGER_THEN_PAGE",pow2_size,0,0);
    }

    if (!root_idx_page[compressed_size]) {
        root_idx_page[compressed_size] = new_idx_page();
    }

    u64 current_idx_entry = 1;
    u64 current_page_data_num = 0;
    volatile u64 *current_idx_page = (volatile u64 *)root_idx_page[compressed_size];
    volatile u64 *current_data_page;

    const u64 max_page_entry = max_cnt_per_page(pow2_size);
    u64 entry_per_page = (max_page_entry + 63) / 64;


    while (TRUE) { //loop for page data.
        if (current_idx_entry>=max_idx_page_bitmap_entrys) { //test if spilled over
            current_idx_entry = 1; //first is used for next idx page
            if (!current_idx_page[0]) { //create new idx page
                current_idx_page[0] = new_idx_page();
            }
            current_idx_page = (volatile u64 *)current_idx_page[0];
        }

        if (!current_idx_page[current_idx_entry]) { //create new data page
            current_idx_page[current_idx_entry] = new_data_page();
        }
        current_data_page = (volatile u64 *)current_idx_page[current_idx_entry];

        current_idx_entry++;

        for (s32 i=0; i<entry_per_page; i++) {
            if (current_idx_entry>=max_idx_page_bitmap_entrys) { //test if spilled over
                current_idx_entry = 1; //first is used for next idx page
                if (!current_idx_page[0]) { //create new idx page
                    current_idx_page[0] = new_idx_page();
                }
                current_idx_page = (volatile u64 *)current_idx_page[0];
            }

            if (current_idx_page[current_idx_entry] != 0xFFFFFFFFFFFFFFFF) {
                s32 bit = __builtin_ctzl(~current_idx_page[current_idx_entry]);
                if (current_idx_page[current_idx_entry] == 0) {
                    bit = 0;
                }
                u64 local_page_num = (i*64) + bit;
                if (local_page_num>=max_page_entry) {
                    current_idx_entry++;
                    continue;
                }

                u64 global_page_num = local_page_num + (max_page_entry * current_page_data_num);

                current_idx_page[current_idx_entry] |= BIT(bit);


                const u64 entry_size = 8 + pow2_size;

                volatile u64 *header = (volatile u64 *)((u64)(current_data_page) + (entry_size * local_page_num));
                *header = (global_page_num * 256) + compressed_size;
                return ((u64)header) + 8;
            }

            current_idx_entry++;
        }
        current_page_data_num++;
    }

    //flip to not
    //find left most 1.
    //use just this, if same use that value if different move to left again then call that.
    //also save value of shunk for which bit it is to be used for store.

    //get left most 1, using flip operator



}

void kernel_allocator_release(u64 location) {
    volatile u64 *header = (volatile u64 *)(location - 8);

    //fetch the page location and which num it is
    u64 compressed_size = *header % 256;
    u64 size = 1UL << compressed_size;
    u64 full_num = *header / 256;
    u32 max_per_page = max_cnt_per_page(size);
    u64 page_num = full_num / max_per_page;
    u64 local_page_num = full_num % max_per_page;

    //fetch idx location
    //first 8 bytes is the location of next idx table


    //extra 1 added is for page location header
    const u64 bitmap_entry_num = (local_page_num/64) + (page_num * (((max_per_page+63)/64)+1)) + 1;
    const u64 bitmap_entry_bit = local_page_num % 64;


    u64 idx_table_num = bitmap_entry_num / max_idx_page_bitmap_entrys;
    u64 idx_table_postion = bitmap_entry_num % max_idx_page_bitmap_entrys;

    volatile u64 *bitmap = (volatile u64 *)root_idx_page[compressed_size];
    for (s32 i=0; i<idx_table_num; i++) { //move to idx paged stored on
        if (!bitmap[0]) {
            PANIC("ALLOCATOR_RELEASE_NO_idx_TABLE_LEAF",0,0,0);
        }
        bitmap = (volatile u64 *)bitmap[0];
    }

    //offset 1 because first is location of next idx page
    bitmap[idx_table_postion + 1] &= ~BIT(bitmap_entry_bit);

    //another place multicore support is lacking
    //needs atomic so can't be edited twice at same time
}

u64 kernel_allocator_bump(u64 size) {
    if (size > KERNEL_PAGE_SIZE) {
        PANIC("ALLOCATOR_BUMP_LARGER_THEN_PAGE",0,0,0);
    }

    if (current_bump_cnt + size >= KERNEL_PAGE_SIZE) {
        current_bump_page = kernel_pager_acquire();
        current_bump_cnt = 0;
    }

    u64 response = current_bump_page + current_bump_cnt;

    current_bump_cnt += ((size+7)/8) * 8;

    return response;
}

//TODO: Add caching so can know where to start looking instant