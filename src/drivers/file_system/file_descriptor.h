#ifndef FILE_DESC_H
#define FILE_DESC_H

#include "drivers/file_system/inode.h"

#define FILE_DESC_READ (1 << 0)
#define FILE_DESC_WRITE (1 << 1)

// Used for mmap and file desc. Will dup on running mmap
typedef struct {
    inode *file;
    u64 seek_pos;
    u8 access_mode;
    u64 virt_mapped_loc; // null if not mmap
} file_descriptor;

file_descriptor *open_file_descriptor(inode *file);
void release_file_descriptor(file_descriptor *desc);
// interuptable function. Must be run with interrupt safe code
u64 int_file_descriptor_read(file_descriptor *desc, u8 *dest, u64 size);
// interuptable function. Must be run with interrupt safe code
u64 int_file_descriptor_write(file_descriptor *desc, u8 *dest, u64 size);

#endif