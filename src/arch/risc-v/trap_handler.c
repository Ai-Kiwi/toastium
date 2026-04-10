#include "panic.h"
#include "arch_irq.h"
#include "safety.h"
#include "kernel_trap.h"
#include "def.h"
#include "scheduler.h"
#include "process.h"

extern char after_trap_hold;

typedef struct {
    unsigned long register_1;
    unsigned long register_2;
    unsigned long register_3;
    unsigned long register_4;
    unsigned long register_5;
    unsigned long register_6;
    unsigned long register_7;
    unsigned long register_8;
    unsigned long register_9;
    unsigned long register_10;
    unsigned long register_11;
    unsigned long register_12;
    unsigned long register_13;
    unsigned long register_14;
    unsigned long register_15;
    unsigned long register_16;
    unsigned long register_17;
    unsigned long register_18;
    unsigned long register_19;
    unsigned long register_20;
    unsigned long register_21;
    unsigned long register_22;
    unsigned long register_23;
    unsigned long register_24;
    unsigned long register_25;
    unsigned long register_26;
    unsigned long register_27;
    unsigned long register_28;
    unsigned long register_29;
    unsigned long register_30;
    unsigned long register_31;
    unsigned long scause; //What caused the trap. (Trap code)
    unsigned long sepc; //What instruction caused trap (Instruction location)
    unsigned long stval; //Extra trap info, e.g page fault says address in question
    unsigned long sstatus; //Privilege level machine was in.
} trap_info;

trap_info arch_processes_trap_info[max_process_count];

void arch_trap_handler(trap_info *trap_data) {//will have pointer input here that points to reg data on stack
    kernel_safety_test();

    const long is_interrupt = trap_data->scause & BIT(63); //last bit
    const long trap_code = (trap_data->scause) & 0x7FFFFFFFFFFFFFFF; //everything but last bit
    const long is_user_mode = trap_data->scause & BIT(8);

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
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code);
            break;
        default:
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code);
            break;
        }
    }else{
        trap.trap_type = KTRAP_TYPE_EXCEPTION;
        switch (trap_code){
        case 0UL: //Instruction address misaligned
            trap.code = KTRAP_ACCESS_MISALIGNED;
            break;
        case 1UL: //Instruction access fault
            trap.code = KTRAP_ACCESS_MISALIGNED;
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
            PANIC("UNHANDLED_TRAP_EXCEPTION_OCCURRED",trap_code);
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

        if (next_process == null_program_pid) {
            trap_data->sepc = (unsigned long)&after_trap_hold; //run a forever wait loop as its idle
        }else{
            *trap_data = arch_processes_trap_info[next_process];
        }
        break;
    case KTRAP_RESPONSE_RESUME_PROCESS:
        if (kernel_running_process == null_program_pid) {
            trap_data->sepc = (unsigned long)&after_trap_hold; //run a forever wait loop as its idle
        }
        break;
    default:
        PANIC("UNHANDLED_TRAP_RESPONSE",trap_response.response_type);
        break;
    }
}