#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include "types.h"

pid kernel_scheduler_next_process();
void kernel_schedular_init();

extern pid kernel_running_process;

#endif