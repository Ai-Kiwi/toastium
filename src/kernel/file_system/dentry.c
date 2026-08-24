#include "dentry.h"
#include "def.h"
#include "dentry.h"
#include "drivers/uart/uart.h"
#include "kernel/file_system/inode.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/list.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"
#include "types.h"

dentry *root_dentry_folder;

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
    if (strcmp(child_name, ".") == 0) {
        return item;
    }
    if (strcmp(child_name, "..") == 0) {
        return item->parent;
    }

    list_iter iter;
    list_iter_create(&item->children, &iter);

    while (iter.cur_upto < iter.cur_list->item_cnt) {
        dentry *child = *(dentry **)list_iter_next(&iter);
        if (strcmp((const char *)&child->name, child_name) == 0) {
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
}

dentry *dentry_get_path(dentry *cwd, char *path) {
    char filename[257];

    char *cur_char = path;
    dentry *cur_node = cwd;

    if (*cur_char == '/') {
        // full path from root
        cur_char++;
        cur_node = root_dentry_folder;
    }

    while (*cur_char != 0x0) {
        char *next_slash = strchr(cur_char, '/');
        if ((u64)next_slash == 0) {
            next_slash = cur_char + strlen(cur_char);
        }

        u64 name_size = (u64)next_slash - (u64)cur_char;

        if (name_size > 256) {
            // name is to large, decline
            return NULL;
        } else {
            // padded higher to get null terminated right
            strscpy(filename, cur_char, 257);
        }

        // doesn't limit how large it copies. It will go past the /

        cur_node = get_child(cur_node, filename);
        if (cur_node == NULL) {
            uart_println_str("no child by name");
            return NULL;
        }

        cur_char = next_slash + 1;
    }

    return cur_node;
}

dentry *create_dentry(dentry *parent, const char *name, inode *file) {
    if (parent != NULL) {
        parent->link_refs += 1;
    }

    dentry *folder = (dentry *)mem_alloc(sizeof(dentry));
    folder->link_refs = 0;
    folder->open_refs = 0;

    if (file == NULL) {
        folder->type = DENTRY_FOLDER;
        list_create(&folder->children, sizeof(u64));
    } else {
        inode_inc_link(file);
        folder->type = DENTRY_OTHER;
        folder->inode = file;
    }

    strscpy(folder->name, name, 256);

    if (parent != NULL) {
        insert_child(parent, folder);
    }

    return folder;
}

void init_dentry() {
    dentry *folder = create_dentry(NULL, "", NULL);
    // prevents being deleted
    folder->link_refs = 100;

    root_dentry_folder = folder;

    // this debug just straght up leaks kernel code really shouldn't be done
    // this way but ok for now for testing as this will definably be removed
    // later.
    const char *text =
        "This is an example text file. This is used currently for debugging vfs. If you also like toast you'd really like the file. not sure why but you would";

    u64 size = strlen(text) + 1;

    inode *example_file = create_blank_inode_file();

    inode_file_debug_inject_data((u64 *)text, size, example_file);

    create_dentry(root_dentry_folder, "example.txt", example_file);
}