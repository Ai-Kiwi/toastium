#include "arch_trap/irq.h"
#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "include/types.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "kernel/trap/page_fault/handler.h"

void handle_async_trap() {
    //need to some how get process
    trap_data trap;
    trapframe_parse((trap_data *)&trap);
    process *proc = (process *)((trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    if (proc->trap_state == PROC_TRAP_PROCESS_PAGE_FAULT) {
        PANIC("PROCESS_TRAP_AFTER_PAGE_FAULT", trap.code, proc->process_id, 0);
    }
    if (proc->trap_state == PROC_TRAP_PROCESS_TRAP) {
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE, (u64)proc->page_fault_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS_PAGE_FAULT;
        if (trap.code != TRAP_PAGE_FAULT) {
            PANIC("DOUBLE_PROCESS_TRAP_NOT_PAGE_FAULT", trap.code, proc->process_id, 0);
        }
    }else{
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE, (u64)proc->kernelspace_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS_TRAP;
    }
    irq_enable();
    u64 queued_response = U64_MAX;
    u64 skip_instruction = FALSE;
    bool8 alive_process = TRUE;

    switch (trap.code) {
        //case KTRAP_SOFTWARE_INTERRUPT:
        //case KTRAP_TIMER_INTERRUPT:
        //case KTRAP_EXTERNAL_INTERRUPT:
        //case KTRAP_ACCESS_MISALIGNED:
        //case KTRAP_ACCESS_FAULT:
        //case KTRAP_INSTRUCTION_INVALID:
        //case KTRAP_BREAKPOINT:
        case TRAP_SYSCALL:
            u64 response = syscall_async_handler(&trap);
            if (response == 1) {
                alive_process = FALSE;
            }
            skip_instruction = TRUE;
            break;
        //case KTRAP_DOUBLE_TRAP:
        //case KTRAP_SOFTWARE_CHECK:
        //case KTRAP_HARDWARE_ERROR:
        case TRAP_PAGE_FAULT:
            bool8 page_loaded = pgfault_load(&trap);
            if (proc->trap_state == PROC_TRAP_PROCESS_PAGE_FAULT) {
                queued_response = !page_loaded;
                if (page_loaded == FALSE) {
                    skip_instruction = TRUE;
                }
            }else{
                //was userspace that called it
                if (page_loaded == FALSE) {
                    uart_println_str("process set to be killed. Invalid page for page fault");
                    alive_process = FALSE;
                }
            }
            break;
        default:
            PANIC("UNHANDLED_KERNEL_PROCESS_TRAP", trap.code, trap.fault_addr, trap.fault_pc);
            break;
    }

    irq_disable();
    if (proc->trap_state == PROC_TRAP_PROCESS_PAGE_FAULT) {
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE, (u64)proc->kernelspace_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS_TRAP;
    }else{
        vma_map_kernel(proc, TRAPFRAME_ADDRESS, KERNEL_PAGE_SIZE, (u64)proc->userspace_trapframe, VMA_READ | VMA_WRITE);
        proc->trap_state = PROC_TRAP_PROCESS;
    }

    //output queued data
    if (queued_response != U64_MAX) {
        trap.return_reg = queued_response;
        trap_data_set_response(&trap);
    }
    if (skip_instruction == TRUE) {
        trap_data_iter_instruction(&trap);
    }
    if (alive_process == FALSE) {
        trap_change_process(&trap);
        kill_process(proc->process_id);
    }
}