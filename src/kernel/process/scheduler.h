#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include "types.h"
#include "kernel/process/process.h"

void kernel_schedular_init();
kernel_process *kernel_scheduler_dequeue_next_process(u64 hart_id);
void kernel_scheduler_queue_process(kernel_process *process);
void kernel_scheduler_dequeue_process(kernel_process *process);

extern pid kernel_running_process;

#endif