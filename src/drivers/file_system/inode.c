#include "drivers/file_system/inode.h"
#include "inode.h"

static void cleanup_file() {

}


void inode_inc_link(inode *file) {
    file->link_refs += 1;
}

void inode_dec_link(inode *file) {
    file->link_refs -= 1;
    if (file->link_refs == 0 && file->open_refs == 0) {
        cleanup_file();
    }
}

void inode_inc_open_cnt(inode *file) {
    file->link_refs += 1;
}

void inode_dec_open_cnt(inode *file) {
    file->link_refs -= 1;
    if (file->link_refs == 0 && file->open_refs == 0) {
        cleanup_file();
    }
}