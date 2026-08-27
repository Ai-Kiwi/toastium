#include "kernel/trap/handler.h"
#include "def.h"
#include "handler.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/process/process.h"

static const u64 page_cnt = ((KERNEL_PAGE_SIZE / 8) - 1);

static void *get_local_fd(u64 local_id, process *proc) {
    u64 page_num = local_id / page_cnt;
    u64 page_loc = local_id % page_cnt;
}

static void insert_local_fd(file_descriptor *fd, process *proc) {
    // loop over all of them and find location.
    // Could possibly store last location that was freed for quicker access.
}

static void remove_local_fd(u64 local_id, process *proc) {
    u64 page_num = local_id / page_cnt;
    u64 page_loc = local_id % page_cnt;
}

// everything in this folder is only interuptable versions.
// As such it isn't split up.

void syscall_fs_open(trap_data *trap) {
    //
}
