#ifndef DENTRY_H
#define DENTRY_H

#include "drivers/file_system/inode.h"
#include "kernel/memory/list.h"

typedef struct {
    char name[256];
    inode *inode;
    list children;
    u32 open_refs; // processes have open folder
    u32 link_refs; // folders are linking to this folder
} dentry;

typedef struct {
    const char name[256];
    dentry *data_ptr;
} dentry_entry;

dentry *get_path(dentry *cwd, char *path);

#endif