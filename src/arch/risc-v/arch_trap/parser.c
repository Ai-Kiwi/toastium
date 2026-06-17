#include "kernel/safety/panic.h"
#include "arch_trap/irq.h"
#include "kernel/safety/safety.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/process/process.h"
#include "kernel/process/scheduler.h"
#include "drivers/uart/uart.h"
#include "include/board.h"
#include "parser.h"

void arch_parse_trap_data(kernel_trap_data trap) {//will have ptr input here that points to reg data on stack
    uart_println_str("kernel trap parser");
    arch_trapframe *trap_frame_data = (arch_trapframe *)TRAPFRAME_ADDRESS;

    const u64 is_interrupt = trap_frame_data->scause & BIT(63); //last bit
    const u64 trap_code = (trap_frame_data->scause) & 0x7FFFFFFFFFFFFFFF; //everything but last bit
    const u64 is_user_mode = trap_frame_data->scause & BIT(8);

    trap.fault_pc = trap_frame_data->sepc;
    trap.fault_addr = trap_frame_data->stval;
    if (is_user_mode) {
        trap.privilege = KTRAP_MODE_USER;
    }else{
        trap.privilege = KTRAP_MODE_SUPERVISOR;
    }

    if (is_interrupt) {
        trap.trap_type = KTRAP_TYPE_INTERRUPT;
        switch (trap_code){
        case 1UL: //Supervisor software interrupt
            trap.code = KTRAP_SOFTWARE_INTERRUPT;
            trap.privilege = KTRAP_MODE_SUPERVISOR;
            break;
        case 3UL: //Machine software interrupt
            trap.code = KTRAP_SOFTWARE_INTERRUPT;
            trap.privilege = KTRAP_MODE_MACHINE;
            break;
        case 5UL: //Supervisor timer interrupt
            trap.code = KTRAP_TIMER_INTERRUPT;
            trap.privilege = KTRAP_MODE_SUPERVISOR;
            break;
        case 7UL: //Machine timer interrupt
            trap.code = KTRAP_TIMER_INTERRUPT;
            trap.privilege = KTRAP_MODE_MACHINE;
            break;
        case 9UL: //Supervisor external interrupt
            trap.code = KTRAP_EXTERNAL_INTERRUPT;
            trap.privilege = KTRAP_MODE_SUPERVISOR;
            break;
        case 11UL: //Machine external interrupt
            trap.code = KTRAP_EXTERNAL_INTERRUPT;
            trap.privilege = KTRAP_MODE_MACHINE;
            break;
        case 13UL: //cnter-overflow interrupt
            //will code support for later when needed
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap.fault_addr, trap.fault_pc);
            break;
        default:
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap.fault_addr, trap.fault_pc);
            break;
        }
    }else{
        trap.trap_type = KTRAP_TYPE_EXCEPTION;
        switch (trap_code){
        case 0UL: //Instruction addr misaligned
            trap.code = KTRAP_ACCESS_MISALIGNED;
            break;
        case 1UL: //Instruction access fault
            trap.code = KTRAP_ACCESS_FAULT;
            break;
        case 2UL: //Illegal instruction
            trap.code = KTRAP_INSTRUCTION_INVALID;
            break;
        case 3UL: //Breakpoint
            trap.code = KTRAP_BREAKPOINT;
            break;
        case 4UL: //Load address misaligned
            trap.code = KTRAP_ACCESS_MISALIGNED;
            break;
        case 5UL: //Load access fault
            trap.code = KTRAP_ACCESS_FAULT;
            break;
        case 6UL: //Store/AMO address misaligned
            trap.code = KTRAP_ACCESS_MISALIGNED;
            break;
        case 7UL: //Store/AMO access fault
            trap.code = KTRAP_ACCESS_FAULT;
            break;
        case 8UL: //Environment call from U-mode
            trap.code = KTRAP_SYSCALL;
            trap.privilege = KTRAP_MODE_USER;
            break;
        case 9UL: //Environment call from S-mode
            trap.code = KTRAP_SYSCALL;
            trap.privilege = KTRAP_MODE_SUPERVISOR;
            break;
        case 11UL: //Environment call from M-mode
            trap.code = KTRAP_SYSCALL;
            trap.privilege = KTRAP_MODE_MACHINE;
            break;
        case 12UL: //Instruction page fault
            trap.code = KTRAP_PAGE_FAULT;
            break;
        case 13UL: //Load page fault
            trap.code = KTRAP_PAGE_FAULT;
            break;
        case 15UL: //Store/AMO page fault
            trap.code = KTRAP_PAGE_FAULT;
            break;
        case 16UL: //Double trap
            trap.code = KTRAP_DOUBLE_TRAP;
            break;
        case 18UL: //Software check
            trap.code = KTRAP_SOFTWARE_CHECK;
            break;
        case 19UL: //Hardware error
            trap.code = KTRAP_HARDWARE_ERROR;
            break;
        default:
            PANIC("UNHANDLED_TRAP_EXCEPTION_OCCURRED",trap_code, trap.fault_addr, trap.fault_pc);
            break;
        }
    }
}