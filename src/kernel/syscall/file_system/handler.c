#include "kernel/trap/handler.h"
#include "arch_trap/irq.h"
#include "def.h"
#include "handler.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/memory/pager.h"
#include "kernel/process/process.h"
#include "kernel/trap/user_accses.h"
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

static u64 fd_insert_local(file_descriptor *fd, process *proc) {
    u64 *page = proc->file_desc_pg;
    u64 page_num = 0;

    while (TRUE) {
        for (u64 i = 0; i < page_cnt; i++) {
            if (page[i + 1] == 0) {
                page[i + 1] = (u64)fd;
                return ((page_num * page_cnt) + i);
            }
        }
        if (page[0] == 0) {
            irq_disable();
            page[0] = pg_alloc();
            irq_enable();
        }
        page = (u64 *)page[0];
        page_num++;
    }
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

    if (cur_page[page_loc] == 0) {
        return;
    }

    cur_page[page_loc] = 0;
}

// everything in this folder is only interuptable versions.
// As such it isn't split up.

void syscall_fs_open(trap_data *trap) {
    process *proc = (process *)trap->process_ptr;

    if (proc->file_desc_cnt >= MAX_OPEN_FILE_DESCRIPTORS) {
        trap->return_reg = -1;
        return;
    }

    u64 path_user_addr = trap->arg1_reg;
    u64 flags = trap->arg2_reg;

    char path[257];
    for (u64 i = 0; i < 257; i++) {
        path[i] = 0;
    }

    bool8 read_success = kernel_read_user(path_user_addr, 256, &path, proc);

    if (path[0] == 0) {
        // nothing read complete failure
        // otherwise could have just been smaller then 256 bytes
        trap->return_reg = -1;
        return;
    }

    file_descriptor *fd = file_open_path(proc->working_dir, path);
    if (fd == NULL) {
        trap->return_reg = -1;
        return;
    }

    u64 local_id = fd_insert_local(fd, proc);
    proc->file_desc_cnt = proc->file_desc_cnt + 1;
    trap->return_reg = local_id;
    return;
}

void syscall_fs_read(trap_data *trap) {
    process *proc = (process *)trap->process_ptr;

    s64 fd_local_id = trap->arg1_reg;
    s64 user_buf_loc = trap->arg2_reg;
    s64 size = trap->arg3_reg;

    file_descriptor *fd = fd_get_local(fd_local_id, proc);
    if (fd == NULL) {
        trap->return_reg = -1;
        return;
    }

    irq_disable();
    u64 *page = (u64 *)pg_alloc();
    irq_enable();

    u64 read_size = 0;

    u64 size_left = size % 4096;
    u64 cur_addr = user_buf_loc;
    for (u64 i = 0; i < (size / 4096); i++) {
        u64 read_cnt = file_descriptor_read(fd, page, 4096);
        read_size = read_size + read_cnt;
        if (read_cnt < 4096) {
            size_left = 0;
            break;
        }
        kernel_write_user(cur_addr, 4096, page, proc);
        cur_addr = cur_addr + 4096;
    }
    if (size_left > 0) {
        u64 read_cnt = file_descriptor_read(fd, page, size_left);
        kernel_write_user(cur_addr, read_cnt, page, proc);
        read_size = read_size + read_cnt;
    }

    irq_disable();
    pg_free((u64)page);
    irq_enable();

    trap->return_reg = read_size;
    return;
}

void syscall_fs_write(trap_data *trap) {
    process *proc = (process *)trap->process_ptr;

    s64 fd_local_id = trap->arg1_reg;
    s64 user_buf_loc = trap->arg2_reg;
    s64 size = trap->arg3_reg;

    file_descriptor *fd = fd_get_local(fd_local_id, proc);
    if (fd == NULL) {
        trap->return_reg = -1;
        return;
    }

    irq_disable();
    u64 *page = (u64 *)pg_alloc();
    irq_enable();

    u64 write_size = 0;

    u64 size_left = size % 4096;
    u64 cur_addr = user_buf_loc;
    for (u64 i = 0; i < (size / 4096); i++) {
        bool8 is_user_read = kernel_read_user(cur_addr, 4096, page, proc);
        if (is_user_read == FALSE) {
            size_left = 0;
            break;
        }

        u64 write_cnt = file_descriptor_write(fd, page, 4096);
        write_size = write_size + write_cnt;
        if (write_cnt < 4096) {
            size_left = 0;
            break;
        }
        cur_addr = cur_addr + 4096;
    }
    if (size_left > 0) {
        bool8 is_user_read = kernel_read_user(cur_addr, size_left, page, proc);
        if (is_user_read == FALSE) {
            size_left = 0;
        }

        u64 write_cnt = file_descriptor_write(fd, page, size_left);
        write_size = write_size + write_cnt;
    }

    irq_disable();
    pg_free((u64)page);
    irq_enable();

    trap->return_reg = write_size;
    return;
}

void syscall_fs_seek(trap_data *trap) {
    process *proc = (process *)trap->process_ptr;

    s64 fd_local_id = trap->arg1_reg;
    u64 seek_pos = trap->arg2_reg;

    file_descriptor *fd = fd_get_local(fd_local_id, proc);
    if (fd == NULL) {
        trap->return_reg = -1;
        return;
    }

    fd->seek_pos = seek_pos;
}