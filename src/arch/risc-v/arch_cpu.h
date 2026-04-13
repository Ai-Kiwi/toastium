
#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include "kernel/trap/irq.h"

void arch_freeze_system();
long arch_get_cycle_count();
long arch_get_time_count();
long arch_set_timer(long cycle_number);

#endif