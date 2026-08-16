#include "kernel/trap/handler.h"
#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "handler.h"
#include "kernel/process/context.h"
#include "kernel/process/process.h"
#include "kernel/process/scheduler.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"
#include "kernel/syscall/handler.h"
#include "kernel/timer/timer.h"
#include "kernel/trap/handler.h"
#include "types.h"
#include "uninterruptible/acces_misaligned.h"
#include "uninterruptible/access_fault.h"
#include "uninterruptible/instruction_invalid.h"
#include "uninterruptible/page_fault.h"
#include "uninterruptible/syscall.h"
#include "uninterruptible/timer.h"

// function returns 0 if it was handled in kernel.
// function returns process kernel stack if its to be handled using process
// stack/kernel

// even if process is changed asm doesn't need to worry (vma swap and trap_frame
// swap done in c)

void trap_change_process(trap_data *trap_data) {
    process *next_process = scheduler_next(trap_data->hart_id);
    context_change_process(next_process, trap_data->hart_id);

    timer_set_future_ms(4);
}

u64 handle_sync_trap() {
    trap_data trap;
    trapframe_parse((trap_data *)&trap);

    process *proc = (process *)trap.process_ptr;
    process_trap_state past_proc_state = proc->trap_state;
    proc->trap_state = PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP;

    u64 response = 0;

    switch (trap.code) {
    case TRAP_ACCESS_MISALIGNED:
        response =
            uninterruptible_trap_access_misaligned(&trap, past_proc_state);
        break;
    case TRAP_ACCESS_FAULT:
        response = uninterruptible_trap_access_fault(&trap, past_proc_state);
        break;
    case TRAP_INSTRUCTION_INVALID:
        response =
            uninterruptible_trap_instruction_invalid(&trap, past_proc_state);
        break;
    case TRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT", trap.privilege,
              trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_SYSCALL:
        response = uninterruptible_trap_syscall(&trap);
        break;
    case TRAP_PAGE_FAULT:
        response = uninterruptible_trap_page_fault(&trap, past_proc_state);
        break;
    case TRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP", trap.privilege, trap.fault_addr,
              trap.fault_pc);
        break;
    case TRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK", trap.privilege,
              trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR", trap.privilege,
              trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT", trap.privilege,
              trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_TIMER_INTERRUPT:
        response = uninterruptible_trap_timer(&trap);
        break;
    case TRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT", trap.privilege,
              trap.fault_addr, trap.fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED", trap.privilege, trap.fault_addr,
              trap.fault_pc);
        break;
    }

    proc->trap_state = past_proc_state;
    return response;
}

// Will return trap action
// Take action first, like kill program or what not. Or other things like add
// disk write to queue Return response, decide if program immediately resumes or
// schedular is run

// if program runs or resumes it depends on trap that was called.
// For some calls like syscalls with for example disk write, there will be non
// block and block operations, this decides if to return or reschedule