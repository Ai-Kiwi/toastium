#include "drivers/file_system/dentry.h"
#include "def.h"
#include "dentry.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/list.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"
#include "types.h"

dentry root_folder;

static void cleanup_dentry(dentry *item) {
    if (item->parent != NULL) {
        dentry *parent = (dentry *)item->parent;
        parent->link_refs -= 1;
    }
    mem_free((u64)item);
    // needs to loop over all children removing them all
}

void dentry_inc_link(dentry *entry) { entry->link_refs += 1; }

void dentry_dec_link(dentry *entry) {
    entry->link_refs -= 1;
    if (entry->link_refs == 0 && entry->open_refs == 0) {
        cleanup_dentry(entry);
    }
}

void dentry_inc_open_cnt(dentry *entry) { entry->link_refs += 1; }

void dentry_dec_open_cnt(dentry *entry) {
    entry->link_refs -= 1;
    if (entry->link_refs == 0 && entry->open_refs == 0) {
        cleanup_dentry(entry);
    }
}

static dentry *get_child(dentry *item, char *child_name) {

    if (strcmp(child_name, ".")) {
        return item;
    }
    if (strcmp(child_name, "..")) {
        return item->parent;
    }

    list_iter iter;
    list_iter_create(&item->children, &iter);

    while (iter.cur_upto < iter.cur_list->item_cnt) {
        dentry *child = *(dentry **)list_iter_next(&iter);
        if (strcmp((const char *)&child->name, child_name) == TRUE) {
            return child;
        }
    }

    return 0;
}

static void remove_child(dentry *child, char *child_name) {

    list_iter iter;
    list_iter_create(&child->children, &iter);

    while (iter.cur_upto < iter.cur_list->item_cnt) {
        dentry *child = *(dentry **)list_iter_next(&iter);
        if (strcmp((const char *)&child->name, child_name) == TRUE) {
            dentry_dec_link(child);
            list_remove(&child->children, iter.cur_upto);
            return;
        }
    }

    PANIC("DENTRY_FAILED_TO_REMOVE_CHILD", (u64)child, 0, 0);
}

static void insert_child(dentry *folder, dentry *dentry_item) {
    u64 item = (u64)dentry_item;
    dentry_inc_link(dentry_item);

    list_append(&folder->children, (u64)&item);

    PANIC("DENTRY_FAILED_TO_REMOVE_CHILD", (u64)folder, 0, 0);
}

dentry *get_path(dentry *cwd, char *path) {
    char filename[257];

    char *cur_char = path;
    dentry *cur_node = cwd;

    if (*cur_char == '/') {
        // full path from root
        cur_char++;
        cur_node = &root_folder;
    }

    while (TRUE) {
        char *next_slash = strchr(cur_char, '/');

        u64 name_size = (u64)next_slash - (u64)cur_char;

        if (name_size > 256) {
            // padded higher to get null terminated right
            strscpy(filename, cur_char, 257);
        } else {
            strncpy(filename, cur_char, name_size);
            filename[name_size] = 0x0;
        }

        // doesn't limit how large it copies. It will go past the /

        cur_node = get_child(cur_node, filename);
        if (cur_node == NULL) {
            return NULL;
        }

        cur_char = next_slash + 1;
    }

    return cur_node;
}

dentry *create_folder(dentry *parent, const char *name) {
    parent->link_refs += 1;

    dentry *folder = (dentry *)mem_alloc(sizeof(dentry));
    folder->link_refs = 0;
    folder->open_refs = 0;

    strscpy(folder->name, name, 256);

    insert_child(parent, folder);

    return folder;
}

void init_dentry() {
    dentry *item = (dentry *)mem_alloc(sizeof(dentry));
    item->name[0] = 0x0;
    item->parent = NULL;
}