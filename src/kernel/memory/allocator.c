#include "kernel/safety/panic.h"
#include "kernel/memory/pager.h"
#include "include/types.h"
#include "include/def.h"

//worth noting, zero is free here, one is in use. This is flipped of pager.

#define max_index_page_bitmap_entrys ((4096 - 8) / 8)

unsigned long root_index_page[256];
unsigned long current_bump_page;
unsigned long current_bump_upto;

unsigned int max_count_per_page(unsigned long size) {
    unsigned long entry_size = size + 8; //add headers
    return 4096 / entry_size;
}

void kernel_allocator_init() {
    for (int i=0; i<256; i++) {
        root_index_page[i] = 0;
    }

    current_bump_page = kernel_pager_acquire();
    current_bump_upto = 0;
}

unsigned long new_index_page() {
    //could make sure everything is zero here, pager already handles this
    unsigned long index_page = (unsigned long)kernel_pager_acquire();
    return index_page;
}

unsigned long new_data_page() {
    //could make sure everything is zero here, pager already handles this
    unsigned long data_page = (unsigned long)kernel_pager_acquire();
    return data_page;
}

unsigned long kernel_allocator_acquire(unsigned long size_bytes) {
    if (!size_bytes) {
        PANIC("ATTEMPT_ALLOCATE_ZERO_BYTES",0,0,0);
    }
    unsigned long leading_zeros = __builtin_clzl(size_bytes);
    unsigned long compressed_size = (63-leading_zeros);
    unsigned long pow2_size = 1UL << compressed_size;
    if (pow2_size != size_bytes) { //round up
        pow2_size = pow2_size << 1;
        compressed_size++;
    }
    if (pow2_size >= 4096) {
        PANIC("ATTEMPT_ALLOCATE_LARGER_THEN_PAGE",pow2_size,0,0);
    }

    if (!root_index_page[compressed_size]) {
        root_index_page[compressed_size] = new_index_page();
    }

    unsigned long current_index_entry = 1;
    unsigned long current_page_data_number = 0;
    volatile unsigned long *current_index_page = (volatile unsigned long *)root_index_page[compressed_size];
    volatile unsigned long *current_data_page;

    const unsigned long max_page_entry = max_count_per_page(pow2_size);
    unsigned long entry_per_page = (max_page_entry + 63) / 64;


    while (TRUE) { //loop for page data.
        if (current_index_entry>=max_index_page_bitmap_entrys) { //test if spilled over
            current_index_entry = 1; //first is used for next index page
            if (!current_index_page[0]) { //create new index page
                current_index_page[0] = new_index_page();
            }
            current_index_page = (volatile unsigned long *)current_index_page[0];
        }

        if (!current_index_page[current_index_entry]) { //create new data page
            current_index_page[current_index_entry] = new_data_page();
        }
        current_data_page = (volatile unsigned long *)current_index_page[current_index_entry];

        current_index_entry++;

        for (int i=0; i<entry_per_page; i++) {
            if (current_index_entry>=max_index_page_bitmap_entrys) { //test if spilled over
                current_index_entry = 1; //first is used for next index page
                if (!current_index_page[0]) { //create new index page
                    current_index_page[0] = new_index_page();
                }
                current_index_page = (volatile unsigned long *)current_index_page[0];
            }

            if (current_index_page[current_index_entry] != 0xFFFFFFFFFFFFFFFF) {
                int bit = __builtin_ctzl(~current_index_page[current_index_entry]);
                if (current_index_page[current_index_entry] == 0) {
                    bit = 0;
                }
                unsigned long local_page_number = (i*64) + bit;
                if (local_page_number>=max_page_entry) {
                    current_index_entry++;
                    continue;
                }

                unsigned long global_page_number = local_page_number + (max_page_entry * current_page_data_number);

                current_index_page[current_index_entry] |= BIT(bit);


                const unsigned long entry_size = 8 + pow2_size;

                volatile unsigned long *header = (volatile unsigned long *)(current_data_page + (entry_size * local_page_number));
                *header = (global_page_number * 256) + compressed_size;

                return (unsigned long)header + 8;
            }

            current_index_entry++;
        }
    }

    //flip to not
    //find left most 1.
    //use just this, if same use that value if different move to left again then call that.
    //also save value of shunk for which bit it is to be used for store.

    //get left most 1, using flip operator



}

void kernel_allocator_release(unsigned long location) {
    volatile unsigned long *header = (volatile unsigned long *)(location - 8);

    //fetch the page location and which number it is
    unsigned long compressed_size = *header % 256;
    unsigned long size = 1UL << compressed_size;
    unsigned long full_number = *header / 256;
    unsigned max_per_page = max_count_per_page(size);
    unsigned long page_number = full_number / max_per_page;
    unsigned long local_page_number = full_number % max_per_page;

    //fetch index location
    //first 8 bytes is the location of next index table


    //extra 1 added is for page location header
    const unsigned long bitmap_entry_number = (local_page_number/64) + (page_number * (((max_per_page+63)/64)+1)) + 1;
    const unsigned long bitmap_entry_bit = local_page_number % 64;


    unsigned long index_table_number = bitmap_entry_number / max_index_page_bitmap_entrys;
    unsigned long index_table_postion = bitmap_entry_number % max_index_page_bitmap_entrys;

    volatile unsigned long *bitmap = (volatile unsigned long *)root_index_page[compressed_size];
    for (int i=0; i<index_table_number; i++) { //move to index paged stored on
        if (!bitmap[0]) {
            PANIC("ALLOCATOR_RELEASE_NO_INDEX_TABLE_LEAF",0,0,0);
        }
        bitmap = (volatile unsigned long *)bitmap[0];
    }

    //offset 1 because first is location of next index page
    bitmap[bitmap_entry_number + 1] &= ~BIT(bitmap_entry_bit);

    //another place multicore support is lacking
    //needs atomic so can't be edited twice at same time
}

unsigned long kernel_allocator_bump(unsigned long size) {
    if (size > 4096) {
        PANIC("ALLOCATOR_BUMP_LARGER_THEN_PAGE",0,0,0);
    }

    if (current_bump_upto + size >= 4096) {
        current_bump_page = kernel_pager_acquire();
        current_bump_upto = 0;
    }

    unsigned long response = current_bump_page + current_bump_upto;

    current_bump_upto + ((size+7)/8);

    return response;
}

//TODO: Add caching so can know where to start looking instant