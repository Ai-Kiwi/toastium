#ifndef KERNEL_LIST_H
#define KERNEL_LIST_H

#include "types.h"

typedef struct {
    u64 *first_section;
    u64 section_size;
    u64 item_cnt;
    u64 item_size;
} list;

typedef struct {
    list *cur_list;
    u64 cur_upto;
    u64 *section_upto;
    u64 max_iter_per_section;
} list_iter;

void list_create(list *new_list, u64 item_size);
u64 list_get(list *new_list, u64 item_idx);
void list_append(list *new_list, u64 src);
void list_remove(list *new_list, u64 item_idx);
void list_delete(list *new_list);
void list_iter_create(list *new_list, list_iter *iter);
u64 list_iter_next(list_iter *iter);

#endif