#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

typedef enum {
    PROC_TYPE_NORMAL,
    PROC_TYPE_IDLE,
    PROC_TYPE_DEAD,
} process_type;

typedef enum {
    PROC_TRAP_PROCESS,
    PROC_TRAP_PROCESS_TRAP,
    PROC_TRAP_PROCESS_PAGE_FAULT
} process_trap_state;

typedef struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    process_trap_state trap_state;
    trapframe *userspace_trapframe;
    trapframe *kernelspace_trapframe;
    trapframe *page_fault_trapframe;
    u64 *vma_table;
    u64 vma_addr_space_id; // in risc-v also known as ASID
    u64 *file_desc_pg; //ptr to pages for process handles. First 4 bytes is location of next. rest are page handle ids.
    u64 file_desc_cnt; //count how mant file descriptors open. Uses swap remove on to many so never missed.
    u16 runing_hart_id;
    bool8 running;
    process_type process_type;
    u64 phys_kernel_stack_addr[4]; //12kb per process kernel stack
} __attribute__((aligned(64))) process;


void processes_init(u64 hart_count);
process *process_from_id(pid process_id);
void processes_iter(void (*function)(u64, u64), u64 parameters);
void process_cleanup(process *process);
void create_init_process();
void kill_process(pid process_id);

#endif