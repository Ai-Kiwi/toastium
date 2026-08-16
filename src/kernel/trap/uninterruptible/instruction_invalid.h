#ifndef KERNEL_TRAP_UNINTERRUPTIBLE_INSTRUCTION_INVALID_H
#define KERNEL_TRAP_UNINTERRUPTIBLE_INSTRUCTION_INVALID_H

#include "kernel/process/process.h"
#include "kernel/trap/handler.h"
#include "types.h"

u64 uninterruptible_trap_instruction_invalid(
    trap_data *trap, process_trap_state past_proc_state);

#endif