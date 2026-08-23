
#include "file_descriptor.h"
#include "arch_trap/irq.h"
#include "drivers/file_system/inode.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/radix.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "utils.h"

file_descriptor *open_file_descriptor(inode *file) {
    inode_inc_open_cnt(file);

    file_descriptor *desc =
        (file_descriptor *)mem_alloc(sizeof(file_descriptor));

    desc->virt_mapped_loc = 0;
    desc->access_mode = 0;
    desc->seek_pos = 0;
    desc->file = file;

    return NULL;
}

void release_file_descriptor(file_descriptor *desc) {
    if (desc->virt_mapped_loc != 0) {
        PANIC("RELEASE_MAPPED_FILE_DESCRIPTOR_NOT_SUPPORTED", 0, 0, 0);
    }

    mem_free((u64)desc);

    inode_dec_open_cnt(desc->file);
}

static inline void increase_offset(u64 count, u64 *offset, u64 *size_left,
                                   u64 *dest_offset) {
    *offset = *offset + 1;
    *size_left = *size_left - 1;
    *dest_offset = *dest_offset + 1;
}

static inline u8 *find_page_start(u64 *offset, inode *file) {
    u64 page_num = *offset / 4096;

    u64 leading_zeros = __builtin_clzl(*offset);
    u64 furthest_one = (63 - leading_zeros);
    u64 radix_level = furthest_one / INODE_FILE_RADIX_LEVEL_DEPTH;

    u8 *page_location = NULL;
    while (page_location == NULL) {
        page_location =
            (u8 *)radix_get(file->file_data.radix_roots[radix_level], page_num,
                            radix_level, INODE_FILE_RADIX_LEVEL_DEPTH);
        if (page_location == NULL) {
            // for now panic, will later load from disk with async.
            PANIC("READ/WRITE_OUT_OF_BOUNDS_FILE_DESC_UNIMPLENTED", 0, 0, 0);
        }
    }
    return page_location;
}

u64 int_file_descriptor_read(file_descriptor *desc, u8 *dest, u64 size) {
    desc;
    //
}

u64 int_file_descriptor_write(file_descriptor *desc, u8 *dest, u64 size) {
    irq_disable();
    inode *file = desc->file;

    switch (file->type) {
    case INODE_SOCKET:
        PANIC("ATTEMPT WRITE TO socket", 0, 0, 0);
        break;
    case INODE_PIPE:
        PANIC("ATTEMPT WRITE TO PIPE", 0, 0, 0);
        break;
    case INODE_FOLDER:
        PANIC("ATTEMPT WRITE TO FOLDER", 0, 0, 0);
        break;
    case INODE_FILE:

        u64 write_size = MIN(size, file->file_data.size - desc->seek_pos);
        u64 size_left = write_size;
        u64 offset = desc->seek_pos;
        u64 dest_offset = 0;

        while (size_left > 0) {
            u8 *page_start = find_page_start(&offset, file);

            u64 page_end = MIN(size_left, 4096);
            page_end = page_end - (offset % 4096);

            for (u64 i = 0; i < page_end; i++) {
                dest[dest_offset] = page_start[offset % 4096];

                increase_offset(1, &offset, &size_left, &dest_offset);
            }
        }

        desc->seek_pos = offset;

        return write_size;
    }
    irq_enable();
    return 0;
}