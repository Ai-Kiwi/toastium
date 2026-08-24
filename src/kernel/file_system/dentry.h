#ifndef DENTRY_H
#define DENTRY_H

#include "inode.h"
#include "kernel/memory/list.h"
#include "types.h"

typedef enum {
    DENTRY_FOLDER,
    DENTRY_OTHER // files, pipes, sockets, etc
} DENTRY_TYPES;

typedef struct {
    char name[256];
    DENTRY_TYPES type;
    union {
        inode *inode;
        list children;
    };
    u32 open_refs; // processes have open folder
    u32 link_refs; // folders are linking to this folder
    void *parent;
} dentry;

extern dentry *root_dentry_folder;

// interuptable function. Must be run with interrupt safe code
dentry *dentry_get_path(dentry *cwd, char *path);
// if file is NULL it is a folder if it is valid it isn't
dentry *create_dentry(dentry *parent, const char *name, inode *file);
void dentry_inc_link(dentry *entry);
void dentry_dec_link(dentry *entry);
void dentry_inc_open_cnt(dentry *entry);
void dentry_dec_open_cnt(dentry *entry);
void init_dentry();

#endif