#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

typedef struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    bool8 in_kernel_space;
    arch_trapframe *userspace_trap_frame;
    arch_trapframe *kernelspace_trap_frame;
    u64 *vma_table;
    u16 runing_hart_id;
    u16 vma_addr_space_id; // in risc-v also known as ASID
} __attribute__((aligned(64))) kernel_process;

void kernel_processes_init(u64 hart_count);
kernel_process *kernel_process_from_id(pid process_id);

#endif