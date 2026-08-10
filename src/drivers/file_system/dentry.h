#ifndef DENTRY_H
#define DENTRY_H

#include "drivers/file_system/inode.h"
#include "kernel/memory/list.h"

typedef struct {
    char name[256];
    inode *inode;
    list children;
} dentry;

typedef struct {
    char name[256];
    dentry *data_ptr;
} dentry_entry;



#endif