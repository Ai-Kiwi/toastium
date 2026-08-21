#include "drivers/file_system/dentry.h"
#include "def.h"
#include "dentry.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/list.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"
#include "types.h"

dentry root_folder;

static void cleanup_dentry(dentry *item) { mem_free((u64)item); }

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

static dentry *get_child(dentry *child, char *child_name) {

    list_iter iter;
    list_iter_create(&child->children, &iter);

    while (iter.cur_upto < iter.cur_list->item_cnt) {
        dentry_list_entry *item = (dentry_list_entry *)list_iter_next(&iter);
        if (str_starts_with((const char *)&item->child_name, child_name) ==
            TRUE) {
            return item->entry;
        }
    }

    return 0;
}

static void remove_child(dentry *child, char *child_name) {

    list_iter iter;
    list_iter_create(&child->children, &iter);

    while (iter.cur_upto < iter.cur_list->item_cnt) {
        dentry_list_entry *item = (dentry_list_entry *)list_iter_next(&iter);
        if (str_starts_with((const char *)&item->child_name, child_name) ==
            TRUE) {
            dentry_dec_link(item->entry);
            list_remove(&child->children, iter.cur_upto);
            return;
        }
    }

    PANIC("DENTRY_FAILED_TO_REMOVE_CHILD", (u64)child, 0, 0);
}

static void insert_child(dentry *folder, dentry *dentry_item,
                         const char *name) {
    dentry_list_entry item;
    item.entry = dentry_item;
    dentry_inc_link(dentry_item);
    strscpy((char *)&item.child_name, name, 256);

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
