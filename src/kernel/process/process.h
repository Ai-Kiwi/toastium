#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "kernel/process/blocks.h"

void init_processes();

typedef struct {
    pid process_id;
    process_block block_waiting;
} process_info;

#endif