#ifndef INODE_H
#define INODE_H

#include "arch_trap/parser.h"
#include "kernel/memory/hashmap.h"
#include "kernel/memory/list.h"
#include "kernel/process/blocks.h"
#include "types.h"

typedef enum {
    INODE_FILE,
    INODE_SOCKET,
    INODE_PIPE,
    INODE_FOLDER,
} inode_type;

#define INODE_FILE_RADIX_LEVEL_DEPTH 4
#define INODE_FILE_RADIX_MAX_DEPTH 16

typedef struct {
    u64 size;
    u64 radix_roots[INODE_FILE_RADIX_MAX_DEPTH];
} inode_file;

typedef struct {
    u32 buf_size;
    u32 data_start;
    u32 data_size;
} inode_pipe;

typedef struct {
    u32 buf_size;
    u32 data_start;
    u32 data_size;
} inode_socket;

typedef struct {
} inode_folder;

typedef struct {
    u32 open_refs; // how many processes have it open
    u32 link_refs; // how many dentry on disk are linking to it
    inode_type type;
    union {
        inode_file file_data;
        inode_pipe pipe_data;
        inode_socket socket_data;
        inode_folder folder_data;
    };
} inode;

void inode_inc_link(inode *file);
void inode_dec_link(inode *file);
void inode_inc_open_cnt(inode *file);
void inode_dec_open_cnt(inode *file);
// used solely for debugging, allows injecting file data before system starts.
// Not for use after system started.
void inode_file_debug_inject_data(u64 *src, u64 size, inode *file);
inode *create_blank_inode_file();

// file descriptor is actually he process accuses to a file

#endif