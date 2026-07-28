#ifndef KERNEL_TRAP_UNINTERRUPTIBLE_TIMER_H
#define KERNEL_TRAP_UNINTERRUPTIBLE_TIMER_H

#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_timer(trap_data *trap);

#endif