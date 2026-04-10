#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include "types.h"

kpid kernel_scheduler_next_process();
void kernel_schedular_init();

extern kpid kernel_running_process;

#endif