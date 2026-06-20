#ifndef KERNEL_PROCESS_CONTEXT_H
#define KERNEL_PROCESS_CONTEXT_H

#include "types.h"
#include "kernel/process/process.h"

void kernel_context_change_process(kernel_process *process, u64 hart_id);
void kernel_context_bootstrap(u64 hart_id);

#endif