
#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include "include/types.h"

void freeze_system();
void idle_loop_system();
u64 cycle_cnt();
u64 time_cnt();
u64 timer_set(u64 cycle_num);

#endif