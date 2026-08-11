#ifndef INODE_H
#define INODE_H

#include "kernel/memory/hashmap.h"
#include "kernel/memory/list.h"
#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

typedef enum {
    INODE_FILE,
    INODE_SOCKET,
    INODE_PIPE,
    INODE_FOLDER,
} inode_type;


typedef struct {
    u64 size;
    u64 radix_roots[16];
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
    u32 open_refs;
    u32 link_refs;
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

//file descriptor is actually he process accuses to a file

#endif