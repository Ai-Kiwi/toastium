#include "kernel/memory/list.h"
#include "def.h"
#include "kernel/memory/pager.h"
#include "kernel/memory/string.h"
#include "kernel/safety/panic.h"
#include "list.h"
#include "types.h"

void list_create(list *new_list, u64 item_size) {
    new_list->section_size = KERNEL_PAGE_SIZE;
    new_list->first_section = (u64 *)pg_alloc();
    new_list->item_cnt = 0;
    new_list->item_size = item_size;
}

u64 list_get(list *new_list, u64 item_idx) { // return ptr to item
    if (item_idx > new_list->item_cnt) {
        PANIC("ATTEMPT_REMOVE_LIST_ITEM_OUT_BOUNDS", 0, 0, 0);
    }

    u64 section_max_cnt = ((new_list->section_size - 8) / new_list->item_size);
    u64 section_num = item_idx / section_max_cnt;
    u64 section_idx = item_idx % section_max_cnt;

    u64 *section = new_list->first_section;

    for (u64 i = 0; i < section_num; i++) {
        section = (u64 *)*section;
        if (section == 0) {
            section = (u64 *)pg_alloc();
        }
    }

    return ((u64)section) + 8 + (new_list->item_size * section_idx);
}

void list_append(list *new_list, u64 src) { // return new item num
    u64 new_item_ptr = list_get(new_list, new_list->item_cnt);

    memcpy(new_item_ptr, src, new_list->item_size);
    new_list->item_cnt += 1;
}

// does swap remove so must move old one
void list_remove(list *new_list, u64 item_idx) {
    if (item_idx >= new_list->item_cnt) {
        PANIC("ATTEMPT_REMOVE_LIST_ITEM_OUT_BOUNDS", 0, 0, 0);
    }

    u64 last_item_ptr = list_get(new_list, new_list->item_size - 1);
    u64 item_removing_ptr = list_get(new_list, item_idx);

    memcpy(item_removing_ptr, last_item_ptr, new_list->item_size);
    new_list->item_cnt -= 1;
}

void list_delete(list *new_list) {
    u64 *section = new_list->first_section;

    while (TRUE) {
        u64 next_section = section[0];
        pg_free((u64)section);
        if (next_section == 0) {
            break;
        }
        section = (u64 *)next_section;
    }
}

void list_iter_create(list *new_list, list_iter *iter) {
    iter->cur_list = new_list;
    iter->cur_upto = 0;
    iter->section_upto = new_list->first_section;
    iter->max_iter_per_section =
        ((new_list->section_size - 8) / new_list->item_size);
}

u64 list_iter_next(list_iter *iter) {
    u64 cur_section_idx = iter->cur_upto % iter->max_iter_per_section;

    u64 ret = ((u64)iter->cur_list->first_section) + 8 +
              (iter->cur_list->item_size * cur_section_idx);

    iter->cur_list += 1;

    // if its a mod of it open next page
    bool8 is_next_section = iter->cur_upto % iter->max_iter_per_section == 0;

    u64 cur_section = (u64)iter->section_upto * (u64)(!is_next_section);
    u64 next_section = iter->section_upto[0] * (u64)is_next_section;

    iter->section_upto = (u64 *)(cur_section + next_section);

    return ret;
}