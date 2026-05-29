#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"
#include "arch_trap/handler.h"

void init_processes();

typedef __attribute__((alignas(64))) struct {
    pid process_id;
    process_block block_waiting; //process needs to be in kernel space for a block to be waiting
    bool8 in_kernel_space;
    arch_trap_state user_trap_state;
    arch_trap_state kernel_trap_state
} process_info;

#endif