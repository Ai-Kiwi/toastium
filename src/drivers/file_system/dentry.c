#include "drivers/file_system/dentry.h"
#include "kernel/memory/list.h"
#include "kernel/safety/panic.h"
#include "types.h"
#include "kernel/memory/string.h"



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


void get_path(dentry *cwd, char *path) {
     
}

