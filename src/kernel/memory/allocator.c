#include "kernel/safety/panic.h"
#include "kernel/memory/pager.h"
#include "include/types.h"
#include "include/def.h"
#include "drivers/uart/uart.h"
#include <stdalign.h>

//worth noting, zero is free here, one is in use. This is flipped of pager.

#define max_idx_page_bitmap_entrys ((KERNEL_PAGE_SIZE - 8) / 8)


typedef struct {
    u64 *page;
    u64 cnt;
} __attribute__((aligned(64))) bump_state;

alignas(64) bump_state bump_allocator_state;
alignas(64) u64 root_idx_page[256]; //multiple of 64, so cache aligned

u32 max_cnt_per_page(u64 size) {
    u64 entry_size = size;
    return (KERNEL_PAGE_SIZE - 64) / entry_size; //adds 64 bytes for page header
}

void kernel_allocator_init() {
    for (s32 i=0; i<256; i++) {
        root_idx_page[i] = 0;
    }

    bump_allocator_state.cnt = 0;
    bump_allocator_state.page = (u64 *)kernel_pager_acquire();
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
    u64 bit_entry_per_page = (max_page_entry + 63) / 64;


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
            u64 *data_page = (u64 *)current_idx_page[current_idx_entry];
            data_page[0] = pow2_size;
            data_page[1] = current_page_data_num;
            data_page[2] = compressed_size;
        }
        current_data_page = (volatile u64 *)current_idx_page[current_idx_entry];

        current_idx_entry++;

        for (s32 i=0; i<bit_entry_per_page; i++) {
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

                if ((current_idx_page[current_idx_entry] & BIT(bit)) != 0UL) {
                    PANIC("DOUBLE_AQUIRE_PAGE", current_idx_entry, global_page_num, 0);
                }

                current_idx_page[current_idx_entry] |= BIT(bit);


                volatile u64 *data = (volatile u64 *)((u64)(current_data_page) + (pow2_size * local_page_num));
                return ((u64)data) + 64; //offset from start
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
    volatile u64 *header = (volatile u64 *)(ROUND_MOD_DOWN(location, KERNEL_PAGE_SIZE));
    volatile u64 *data = (volatile u64 *)(location);

    //fetch the page location and which num it is
    u64 entry_size = header[0];
    u64 page_num = header[1];
    u64 compressed_size = header[2];
    u32 max_per_page = max_cnt_per_page(entry_size);
    u64 page_entry_num = (((u64)&data) - ((u64)&header - 64)) / entry_size;

    //fetch idx location
    //first 8 bytes is the location of next idx table


    //extra 1 added is for page location header
    const u64 bitmap_entry_num = (page_entry_num/64) + (page_num * (ROUND_MOD_UP(max_per_page,64)+1));
    const u64 bitmap_entry_bit = page_entry_num % 64;

    //convert
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

    if (bump_allocator_state.cnt + size >= KERNEL_PAGE_SIZE) {
        bump_allocator_state.page = (u64 *)kernel_pager_acquire();
        bump_allocator_state.cnt = 0;
    }

    u64 response = ((u64)bump_allocator_state.page) + bump_allocator_state.cnt;

    bump_allocator_state.cnt += ROUND_MOD_UP(size, 8);

    return response;
}

//TODO: Add caching so can know where to start looking instant
