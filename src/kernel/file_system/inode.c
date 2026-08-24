#include "inode.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "inode.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/pager.h"
#include "kernel/memory/radix.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"

static void cleanup_file(inode *file) { mem_free((u64)file); }

void inode_inc_link(inode *file) { file->link_refs += 1; }

void inode_dec_link(inode *file) {
    file->link_refs -= 1;
    if (file->link_refs == 0 && file->open_refs == 0) {
        cleanup_file(file);
    }
}

void inode_inc_open_cnt(inode *file) { file->link_refs += 1; }

void inode_dec_open_cnt(inode *file) {
    file->link_refs -= 1;
    if (file->link_refs == 0 && file->open_refs == 0) {
        cleanup_file(file);
    }
}

inode *create_blank_inode_file() {
    inode *file = (inode *)mem_alloc(sizeof(inode));
    file->type = INODE_FILE;
    file->link_refs = 0;
    file->open_refs = 0;
    file->file_data.size = 0;

    u64 cur_radix = 0;

    for (u64 i = 0; i < INODE_FILE_RADIX_MAX_DEPTH; i++) {
        u64 new_radix = radix_create(INODE_FILE_RADIX_LEVEL_DEPTH);

        radix_insert(new_radix, 0, cur_radix, 1, INODE_FILE_RADIX_LEVEL_DEPTH);
        cur_radix = new_radix;
        file->file_data.radix_roots[i] = cur_radix;
    }

    return file;
}

void inode_file_debug_inject_data(u64 *src, u64 size, inode *file) {
    if (file->type != INODE_FILE) {
        PANIC("inode_file_debug_inject_data_ON_NON_FILE", 0, 0, 0);
    }

    u8 *src_u8 = (u8 *)src;

    u64 page_cnt = ROUND_MOD_UP(size, 4096) / 4096;

    for (u64 i = 0; i < page_cnt; i++) {
        u64 *page_addr = (u64 *)pg_alloc();
        u8 *page_addr_u8 = (u8 *)page_addr;

        if (i != page_cnt - 1) {
            for (u64 j = 0; j < 512; j++) {
                page_addr[j] = src[(512 * i) + j];
            }
        } else {
            const u64 u8_left_size = size % 4096;
            for (u64 j = 0; j < u8_left_size; j++) {
                page_addr_u8[j] = src_u8[(4096 * i) + j];
            }
        }

        // inefficient but this is a debug program so fine for now.
        u64 old_addr = radix_insert(
            file->file_data.radix_roots[INODE_FILE_RADIX_MAX_DEPTH - 1], i,
            (u64)page_addr, INODE_FILE_RADIX_MAX_DEPTH,
            INODE_FILE_RADIX_LEVEL_DEPTH);
        if (old_addr) {
            PANIC("inode_file_debug_inject_data_ALREADY_CONTENTS", 0, 0, 0);
        }
    }
    file->file_data.size = size;
}