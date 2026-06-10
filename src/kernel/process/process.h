#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/parser.h"

void init_processes();

typedef struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    bool8 in_kernel_space;
    arch_trapframe *userspace_trap_frame;
    arch_trapframe *kernelspace_trap_frame;
    u64 *vma_table;
    u16 asid; // in risc-v also known as ASID
} __attribute__((aligned(64))) kernel_process;

#endif