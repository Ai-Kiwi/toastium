#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include "types.h"
#include "kernel/process/process.h"

void schedular_init(u64 hart_count);
process *scheduler_next(u64 hart_id);
void scheduler_queue_process(process *process);
void scheduler_remove(process *process);

extern pid kernel_running_process;

#endif