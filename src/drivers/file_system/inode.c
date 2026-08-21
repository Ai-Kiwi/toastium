#include "drivers/file_system/inode.h"
#include "inode.h"
#include "kernel/memory/allocator.h"

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