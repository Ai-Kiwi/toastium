#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

typedef enum {
    KPROC_TYPE_NORMAL,
    KPROC_TYPE_IDLE,
    KPROC_TYPE_DEAD,
} kernel_process_type;

typedef enum {
    KPROC_TRAP_PROCESS,
    KPROC_TRAP_PROCESS_TRAP,
    KPROC_TRAP_PROCESS_PAGE_FAULT
} process_trap_state;

typedef struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    process_trap_state trap_state;
    arch_trapframe *userspace_trap_frame;
    arch_trapframe *kernelspace_trap_frame;
    arch_trapframe *page_fault_trap_frame;
    u64 *vma_table;
    u64 vma_addr_space_id; // in risc-v also known as ASID
    u64 *handles_page; //ptr to pages for process handles. First 4 bytes is location of next. rest are page handle ids.
    u64 kernel_stack;
    u16 runing_hart_id;
    bool8 running;
    kernel_process_type process_type;
} __attribute__((aligned(64))) kernel_process;


void kernel_processes_init(u64 hart_count);
kernel_process *kernel_process_from_id(pid process_id);
void kernel_process_iter(void (*function)(u64, u64), u64 parameters);
void kernel_process_cleanup_process(kernel_process *process);
void kernel_process_create_init_process();
void kernel_process_kill_process(pid process_id);

#endif