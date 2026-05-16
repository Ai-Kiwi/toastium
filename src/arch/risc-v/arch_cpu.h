
#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include "kernel/trap/irq.h"
#include "include/types.h"

void arch_freeze_system();
u64 arch_get_cycle_count();
u64 arch_get_time_count();
u64 arch_set_timer(u64 cycle_number);

#endif