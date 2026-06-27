#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "types.h"

typedef struct { //global file data
    u64 path_len;
    char *path;
    u64 size;
    u64 *radix_roots[16];
    u64 handle_count; //how many handles across all files are using this
} __attribute__((aligned(64))) file;

typedef struct { //per process
    u64 seek_postion;
    file *file_ptr;
} file_handle;

#endif

