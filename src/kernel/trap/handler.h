#ifndef KERNEL_TRAP_H
#define KERNEL_TRAP_H

#include "types.h"
typedef enum {
    KTRAP_MODE_USER,
    KTRAP_MODE_SUPERVISOR,
    KTRAP_MODE_MACHINE,
} privilege_mode;

typedef enum {
    KTRAP_TYPE_INTERRUPT,
    KTRAP_TYPE_EXCEPTION,
} kernel_trap_type;

typedef enum {
    //interrupts
    KTRAP_SOFTWARE_INTERRUPT,
    KTRAP_TIMER_INTERRUPT,
    KTRAP_EXTERNAL_INTERRUPT,
    //COUNTER_OVERFLOW_INTERRUPT, unhandled for now until needed

    //Exception
    KTRAP_ACCESS_MISALIGNED,
    KTRAP_ACCESS_FAULT,
    KTRAP_INSTRUCTION_INVALID,
    KTRAP_BREAKPOINT,
    KTRAP_SYSCALL,
    KTRAP_PAGE_FAULT,
    KTRAP_DOUBLE_TRAP,
    KTRAP_SOFTWARE_CHECK,
    KTRAP_HARDWARE_ERROR,

} kernel_trap_code;

typedef struct {
    kernel_trap_type trap_type;
    privilege_mode privilege;
    kernel_trap_code code;
    u64 fault_pc;
    u64 fault_address;
} kernel_trap_data;

typedef enum {
    KTRAP_RESPONSE_RESUME_PROCESS,
    KTRAP_RESPONSE_HOLD_PROCESS
} kernel_trap_response_type;

typedef struct {
    kernel_trap_response_type response_type;
    kpid kernel_PID;
} kernel_trap_response;

const kernel_trap_response kernel_handle_trap(const kernel_trap_data *trap_data);

#endif