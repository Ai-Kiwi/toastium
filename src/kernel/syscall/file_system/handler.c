#include "kernel/trap/handler.h"
#include "def.h"
#include "handler.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/process/process.h"
#include "types.h"

static const u64 page_cnt = ((KERNEL_PAGE_SIZE / 8) - 1);

static file_descriptor *fd_get_local(u64 local_id, process *proc) {
    u64 page_num = local_id / page_cnt;
    u64 page_loc = local_id % page_cnt;

    u64 *cur_page = proc->file_desc_pg;
    for (u64 i = 0; i < page_num; i++) {
        cur_page = (u64 *)cur_page[0];
        if (cur_page == NULL) {
            return NULL;
        }
    }

    // if not set would already return null
    return (file_descriptor *)cur_page[page_loc + 2];
}

static void fd_insert_local(file_descriptor *fd, process *proc) {
    // loop over all of them and find location.
    // Could possibly store last location that was freed for quicker access.
}

static void fd_remove_local(u64 local_id, process *proc) {
    u64 page_num = local_id / page_cnt;
    u64 page_loc = (local_id % page_cnt) + 1; // offset 1

    u64 *cur_page = proc->file_desc_pg;
    for (u64 i = 0; i < page_num; i++) {
        cur_page = (u64 *)cur_page[0];
        if (cur_page == NULL) {
            return;
        }
    }

    if (cur_page[page_loc + 2] == 0) {
        return;
    }

    cur_page[page_loc + 2] = 0;
}

// everything in this folder is only interuptable versions.
// As such it isn't split up.

void syscall_fs_open(trap_data *trap) {}
