#include "kernel/safety/panic.h"
#include "arch_trap/irq.h"
#include "kernel/safety/safety.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/process/process.h"
#include "kernel/process/scheduler.h"
#include "drivers/uart/uart.h"

typedef struct {
    u64 register_1;
    u64 register_2;
    u64 register_3;
    u64 register_4;
    u64 register_5;
    u64 register_6;
    u64 register_7;
    u64 register_8;
    u64 register_9;
    u64 register_10;
    u64 register_11;
    u64 register_12;
    u64 register_13;
    u64 register_14;
    u64 register_15;
    u64 register_16;
    u64 register_17;
    u64 register_18;
    u64 register_19;
    u64 register_20;
    u64 register_21;
    u64 register_22;
    u64 register_23;
    u64 register_24;
    u64 register_25;
    u64 register_26;
    u64 register_27;
    u64 register_28;
    u64 register_29;
    u64 register_30;
    u64 register_31;
    u64 scause; //What caused the trap. (Trap code)
    u64 sepc; //What instruction caused trap (Instruction location)
    u64 stval; //Extra trap info, e.g page fault says address in question
    u64 sstatus; //Privilege level machine was in.
} trap_info;

trap_info arch_processes_trap_info[max_process_count];

void arch_trap_handler(trap_info *trap_data) {//will have pointer input here that points to reg data on stack
    kernel_safety_test();

    const u64 is_interrupt = trap_data->scause & BIT(63); //last bit
    const u64 trap_code = (trap_data->scause) & 0x7FFFFFFFFFFFFFFF; //everything but last bit
    const u64 is_user_mode = trap_data->scause & BIT(8);

    kernel_trap_data trap;
    trap.fault_pc = trap_data->sepc;
    trap.fault_address = trap_data->stval;
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
        case 13UL: //Counter-overflow interrupt
            //will code support for later when needed
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap.fault_address, trap.fault_pc);
            break;
        default:
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap.fault_address, trap.fault_pc);
            break;
        }
    }else{
        trap.trap_type = KTRAP_TYPE_EXCEPTION;
        switch (trap_code){
        case 0UL: //Instruction address misaligned
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
            PANIC("UNHANDLED_TRAP_EXCEPTION_OCCURRED",trap_code, trap.fault_address, trap.fault_pc);
            break;
        }
    }

    kernel_trap_response trap_response = kernel_handle_trap(&trap);

    switch (trap_response.response_type){
    case KTRAP_RESPONSE_HOLD_PROCESS:
        //need to swap out for another process
        if (kernel_running_process != null_program_pid) {
            arch_processes_trap_info[kernel_running_process] = *trap_data;
        }

        kpid next_process = kernel_scheduler_next_process();

        *trap_data = arch_processes_trap_info[next_process];
        break;
    case KTRAP_RESPONSE_RESUME_PROCESS:
        break;
    default:
        PANIC("UNHANDLED_TRAP_RESPONSE",trap_response.response_type, trap_response.kernel_PID, 0);
        break;
    }
}