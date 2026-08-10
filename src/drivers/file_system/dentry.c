#include "drivers/file_system/dentry.h"
#include "kernel/memory/list.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/memory/string.h"

dentry root_folder;



dentry *get_child(dentry *cwd, char *name) {

    list_iter iter;
    list_iter_create(&cwd->children, &iter);


    while (iter.cur_upto<iter.cur_list->item_cnt) {
        dentry_entry *entry = (dentry_entry *)list_iter_next(&iter);
        if (str_starts_with((const char *)&entry->name, (const char *)name) == TRUE) {
            return entry->data_ptr;
        }
    }


    return 0;
}

void remove_child(dentry *cwd, char *name) {

    list_iter iter;
    list_iter_create(&cwd->children, &iter);


    while (iter.cur_upto<iter.cur_list->item_cnt) {
        dentry_entry *entry = (dentry_entry *)list_iter_next(&iter);
        if (str_starts_with((const char *)&entry->name, (const char *)name) == TRUE) {
            list_remove(&cwd->children, iter.cur_upto);
            return;
        }
    }

    PANIC("DENTRY_FAILED_TO_REMOVE_CHILD", (u64)cwd, 0, 0);
}

void insert_child(dentry *cwd, char *name) {
    dentry_entry entry;
    strncpy((char *)&entry.name, (const char *)&name, 256);
    list_append(&cwd->children, (u64)&entry);

    PANIC("DENTRY_FAILED_TO_REMOVE_CHILD", (u64)cwd, 0, 0);
}


dentry *get_path(dentry *cwd, char *path) {
    char filename[257];

    char *cur_char = path;
    dentry *cur_node = cwd;

    if (*cur_char == '/') {
        //full path from root
        cur_char++;
        cur_node = &root_folder;
    }

    while (TRUE) {
        char *next_slash = strchr(cur_char, '/');

        u64 name_len = (u64)next_slash - (u64)cur_char - 1;

        strncpy(filename, cur_char, 257);//padded higher so it will always end in being null terminated

        cur_node = get_child(cur_node, filename);
        if (cur_node == NULL) {
            return NULL;
        }

        cur_char = next_slash + 1;
    }

    return cur_node;
}

