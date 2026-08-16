#ifndef KERNEL_TRAP_UNINTERRUPTIBLE_ACCESS_MISALIGNED_H
#define KERNEL_TRAP_UNINTERRUPTIBLE_ACCESS_MISALIGNED_H

#include "kernel/process/process.h"
#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_access_misaligned(trap_data *trap,
                                           process_trap_state past_proc_state);

#endif