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

void kernel_trap_process_kernel() {
    //need to some how get process
    kernel_trap_data trap_data;
    arch_parse_trap_data((kernel_trap_data *)&trap_data);
    kernel_process *process = (kernel_process *)((arch_trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    if (process->trap_state == KPROC_TRAP_PROCESS_PAGE_FAULT) {
        PANIC("PROCESS_TRAP_AFTER_PAGE_FAULT", trap_data.code, process->process_id, 0);
    }
    if (process->trap_state == KPROC_TRAP_PROCESS_TRAP) {
        arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->page_fault_trap_frame, VMA_READ | VMA_WRITE);
        process->trap_state = KPROC_TRAP_PROCESS_PAGE_FAULT;
        if (trap_data.code != KTRAP_PAGE_FAULT) {
            PANIC("DOUBLE_PROCESS_TRAP_NOT_PAGE_FAULT", trap_data.code, process->process_id, 0);
        }
    }else{
        arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->kernelspace_trap_frame, VMA_READ | VMA_WRITE);
        process->trap_state = KPROC_TRAP_PROCESS_TRAP;
    }
    arch_irq_enable();
    u64 queued_response = U64_MAX;
    u64 skip_instruction = FALSE;
    bool8 kill_process = FALSE;

    switch (trap_data.code) {
        //case KTRAP_SOFTWARE_INTERRUPT:
        //case KTRAP_TIMER_INTERRUPT:
        //case KTRAP_EXTERNAL_INTERRUPT:
        //case KTRAP_ACCESS_MISALIGNED:
        //case KTRAP_ACCESS_FAULT:
        //case KTRAP_INSTRUCTION_INVALID:
        //case KTRAP_BREAKPOINT:
        case KTRAP_SYSCALL:
            u64 response = kernel_syscall_async_handler(&trap_data);
            if (response == 1) {
                kill_process = TRUE;
            }
            skip_instruction = TRUE;
            break;
        //case KTRAP_DOUBLE_TRAP:
        //case KTRAP_SOFTWARE_CHECK:
        //case KTRAP_HARDWARE_ERROR:
        case KTRAP_PAGE_FAULT:
            bool8 page_loaded = kernel_page_fault_load();
            if (process->trap_state == KPROC_TRAP_PROCESS_PAGE_FAULT) {
                queued_response = !page_loaded;
                if (page_loaded == FALSE) {
                    skip_instruction = TRUE;
                }
            }else{
                //was userspace that called it
                if (page_loaded == FALSE) {
                    uart_println_str("process set to be killed. Invalid page for page fault");
                    kill_process = TRUE;
                }
            }
            break;
        default:
            PANIC("UNHANDLED_KERNEL_PROCESS_TRAP", trap_data.code, trap_data.fault_addr, trap_data.fault_pc);
            break;
    }

    arch_irq_disable();
    if (process->trap_state == KPROC_TRAP_PROCESS_PAGE_FAULT) {
        arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->kernelspace_trap_frame, VMA_READ | VMA_WRITE);
        process->trap_state = KPROC_TRAP_PROCESS_TRAP;
    }else{
        arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->userspace_trap_frame, VMA_READ | VMA_WRITE);
        process->trap_state = KPROC_TRAP_PROCESS;
    }

    //output queued data
    if (queued_response != U64_MAX) {
        trap_data.return_reg = queued_response;
        arch_trap_set_response(&trap_data);
    }
    if (skip_instruction == TRUE) {
        arch_trap_iter_instruction(&trap_data);
    }
    if (kill_process == TRUE) {
        kernel_trap_change_process(&trap_data);
        kernel_process_kill_process(process->process_id);
    }
}