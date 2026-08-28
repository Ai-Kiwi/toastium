#ifndef KERNEL_TRAP_H
#define KERNEL_TRAP_H

#include "types.h"
typedef enum {
    TRAP_MODE_USER,
    TRAP_MODE_SUPERVISOR,
    TRAP_MODE_MACHINE,
} privilege_mode;

typedef enum {
    TRAP_TYPE_INTERRUPT,
    TRAP_TYPE_EXCEPTION,
} trap_type;

typedef enum {
    // interrupts
    TRAP_SOFTWARE_INTERRUPT,
    TRAP_TIMER_INTERRUPT,
    TRAP_EXTERNAL_INTERRUPT,
    // cntER_OVERFLOW_INTERRUPT, unhandled for now until needed

    // Exception
    TRAP_ACCESS_MISALIGNED,
    TRAP_ACCESS_FAULT,
    TRAP_INSTRUCTION_INVALID,
    TRAP_BREAKPOINT,
    TRAP_SYSCALL,
    TRAP_PAGE_FAULT,
    TRAP_DOUBLE_TRAP,
    TRAP_SOFTWARE_CHECK,
    TRAP_HARDWARE_ERROR,

} trap_code;

typedef struct {
    u64 process_ptr; // would like to be a process pointer type but causes
                     // circular import, will revisit
    trap_type trap_type;
    privilege_mode privilege;
    trap_code code;
    u64 fault_pc;
    u64 fault_addr;
    u64 arg0_reg;
    u64 arg1_reg;
    u64 arg2_reg;
    u64 arg3_reg;
    s64 return_reg;
    u64 hart_id;
} trap_data;

typedef enum {
    TRAP_RET_RESUME_PROCESS,
    TRAP_RET_HOLD_PROCESS
} trap_response_type;

u64 handle_sync_trap();
void trap_change_process(trap_data *trap_data);

#endif