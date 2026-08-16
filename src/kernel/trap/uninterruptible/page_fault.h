#ifndef KERNEL_TRAP_UNINTERRUPTIBLE_PAGE_FAULT_H
#define KERNEL_TRAP_UNINTERRUPTIBLE_PAGE_FAULT_H

#include "kernel/process/process.h"
#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_page_fault(trap_data *trap,
                                    process_trap_state past_proc_state);

#endif