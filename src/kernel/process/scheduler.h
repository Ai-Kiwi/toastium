#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include "types.h"

pid kernel_scheduler_next_process();
void kernel_schedular_init();
void kernel_schedular_bootstrap(u64 hart_id);

extern pid kernel_running_process;

#endif