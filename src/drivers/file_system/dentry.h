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
    void *parent;
} dentry;

dentry *get_path(dentry *cwd, char *path);

void dentry_inc_link(dentry *entry);
void dentry_dec_link(dentry *entry);
void dentry_inc_open_cnt(dentry *entry);
void dentry_dec_open_cnt(dentry *entry);

#endif