#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/handler.h"
#include <arch_trap/parser.h>

void init_processes();

typedef struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    bool8 in_kernel_space;
    arch_trap_frame user_trap_state;
    arch_trap_frame kernel_trap_state;
    u64 *virtual_memory_root_table;
    u16 virtual_memory_number; // in risc-v also known as ASID
} __attribute__((aligned(64))) process_info;

#endif