#include "kernel/trap/process_handler.h"
#include "arch_trap/irq.h"
#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "include/types.h"
#include "interruptible/page_fault.h"
#include "interruptible/syscall.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "kernel/trap/page_fault/handler.h"
#include "process_handler.h"

void handle_async_trap() {
    // need to some how get process
    trap_data trap;
    trapframe_parse((trap_data *)&trap);
    process *proc = (process *)trap.process_ptr;
    process_trap_state past_proc_state = proc->trap_state;
    switch (past_proc_state) {
    case PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP:
        PANIC("INTERRUPTABLE_TRAP_ENTERED_FROM_UNINTERRUPTABLE", 0, 0, 0);
        break;
    case PROC_TRAP_PROCESS_INTERRUPTABLE_TRAP:
        // must be attempting to read userspace
        if (trap.code != TRAP_PAGE_FAULT || proc->reading_userspace == FALSE) {
            PANIC("DOUBLE_PROCESS_TRAP_NOT_PAGE_FAULT", trap.code,
                  proc->process_id, 0);
        }
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE,
                       (u64)proc->page_fault_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS_READ_USERSPACE;
        break;
    case PROC_TRAP_PROCESS:
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE,
                       (u64)proc->kernelspace_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS_INTERRUPTABLE_TRAP;
        break;
    case PROC_TRAP_PROCESS_READ_USERSPACE:
        PANIC("PAGE_FAULT_FROM_KERNEL_READ_USERSPACE", trap.code,
              proc->process_id, 0);
        break;
        break;
    }
    irq_enable();
    interruptable_trap_response response;
    response.queued_response = 0;
    response.send_response = FALSE;
    response.kill_process = FALSE;
    response.skip_instruction = FALSE;

    switch (trap.code) {
    // case KTRAP_SOFTWARE_INTERRUPT:
    // case KTRAP_TIMER_INTERRUPT:
    // case KTRAP_EXTERNAL_INTERRUPT:
    // case KTRAP_ACCESS_MISALIGNED:
    // case KTRAP_ACCESS_FAULT:
    // case KTRAP_INSTRUCTION_INVALID:
    // case KTRAP_BREAKPOINT:
    case TRAP_SYSCALL:
        interruptable_trap_syscall(&trap, &response);
        break;
    // case KTRAP_DOUBLE_TRAP:
    // case KTRAP_SOFTWARE_CHECK:
    // case KTRAP_HARDWARE_ERROR:
    case TRAP_PAGE_FAULT:
        interruptable_trap_page_fault(&trap, &response);
        break;
    default:
        PANIC("UNHANDLED_KERNEL_PROCESS_TRAP", trap.code, trap.fault_addr,
              trap.fault_pc);
        break;
    }

    irq_disable();
    switch (past_proc_state) {
    case PROC_TRAP_PROCESS:
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE,
                       (u64)proc->userspace_trapframe, VMA_READ | VMA_WRITE);
        break;
    case PROC_TRAP_PROCESS_INTERRUPTABLE_TRAP:
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE,
                       (u64)proc->kernelspace_trapframe, VMA_READ | VMA_WRITE);
        break;
    case PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP:
        PANIC(
            "UNEXPECTED_UNINTERUPTABLE_EXIT_WITH_PROC_TRAP_PROCESS_UNINTERRUPTABLE_TRAP",
            0, 0, 0);
        break;
    case PROC_TRAP_PROCESS_READ_USERSPACE: // can't happen
        PANIC(
            "UNEXPECTED_UNINTERUPTABLE_EXIT_WITH_PROC_TRAP_PROCESS_READ_USERSPACE",
            0, 0, 0);
        break;
        break;
    }

    proc->trap_state = past_proc_state;

    // output queued data
    if (response.send_response == TRUE) {
        trap.return_reg = response.queued_response;
        trap_data_set_response(&trap);
    }
    if (response.skip_instruction == TRUE) {
        trap_data_iter_instruction(&trap);
    }
    if (response.kill_process == TRUE) {
        trap_change_process(&trap);
        kill_process(proc->process_id);
    }
}