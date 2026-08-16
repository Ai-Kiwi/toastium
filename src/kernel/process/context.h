#ifndef KERNEL_PROCESS_CONTEXT_H
#define KERNEL_PROCESS_CONTEXT_H

#include "kernel/process/process.h"
#include "types.h"

void context_change_process(process *process, u64 hart_id);
void context_bootstrap(u64 hart_id);

#endif