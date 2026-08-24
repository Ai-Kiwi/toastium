#ifndef PROCESS_H
#define PROCESS_H

#include "arch_trap/parser.h"
#include "kernel/file_system/dentry.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/memory/hashmap.h"
#include "kernel/memory/list.h"
#include "kernel/process/blocks.h"
#include "types.h"

typedef enum {
    PROC_TYPE_NORMAL,
    PROC_TYPE_IDLE,
    PROC_TYPE_DEAD,
} process_type;

typedef enum {
    PROC_TRAP_PROCESS,
    PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP,
    PROC_TRAP_PROCESS_INTERRUPTABLE_TRAP,
    PROC_TRAP_PROCESS_READ_USERSPACE
} process_trap_state;

typedef struct {
    u64 process_upto;
    hashmap process_hashmap;
    list running_list;
} __attribute__((aligned(64))) process_handler_state;

typedef struct {
    pid process_id;
    // process needs to be in kernel space for a block to be waiting
    process_block block_waiting;
    process_trap_state trap_state;
    trapframe *userspace_trapframe;
    trapframe *kernelspace_trapframe;
    trapframe *page_fault_trapframe;
    u64 *vma_table;
    // in risc-v also known as ASID
    u64 vma_addr_space_id;
    // ptr to pages for process handles. First 4 bytes is location of next. rest
    // are page handle ids.
    u64 *file_desc_pg;
    // count how mant file descriptors open. Uses swap remove so never missed.
    u64 file_desc_cnt;
    u16 runing_hart_id;
    bool8 running;
    file_descriptor *exe_file;
    dentry *working_dir;
    process_type process_type;
    u64 phys_kernel_stack_addr[4]; // 12kb per process kernel stack
    u64 list_idx;
    bool8 reading_userspace;
} __attribute__((aligned(64))) process;

void processes_init(u64 hart_count);
process *process_from_id(pid process_id);
void process_cleanup(process *process);
void create_init_process();
void kill_process(pid process_id);
void processes_iter(list_iter *iter);

#endif