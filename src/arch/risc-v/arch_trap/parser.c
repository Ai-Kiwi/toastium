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

void trapframe_parse(trap_data *trap) {//will have ptr input here that points to reg data on stack
    trapframe *trap_frame_data = (trapframe *)TRAPFRAME_ADDRESS;

    const u64 is_interrupt = trap_frame_data->scause & BIT(63); //last bit
    const u64 trap_code = (trap_frame_data->scause) & 0x7FFFFFFFFFFFFFFF; //everything but last bit
    const u64 is_user_mode = trap_frame_data->scause & BIT(8);

    trap->process_ptr = trap_frame_data->process_ptr;

    trap->arg0_reg = trap_frame_data->register_10;
    trap->arg1_reg = trap_frame_data->register_11;
    trap->arg2_reg = trap_frame_data->register_12;
    trap->arg3_reg = trap_frame_data->register_13;
    trap->return_reg = trap_frame_data->register_10;

    trap->hart_id = trap_frame_data->hart_id;

    trap->fault_pc = trap_frame_data->sepc;
    trap->fault_addr = trap_frame_data->stval;
    if (is_user_mode) {
        trap->privilege = TRAP_MODE_USER;
    }else{
        trap->privilege = TRAP_MODE_SUPERVISOR;
    }

    if (is_interrupt) {
        trap->trap_type = TRAP_TYPE_INTERRUPT;
        switch (trap_code){
        case 1UL: //Supervisor software interrupt
            trap->code = TRAP_SOFTWARE_INTERRUPT;
            trap->privilege = TRAP_MODE_SUPERVISOR;
            break;
        case 3UL: //Machine software interrupt
            trap->code = TRAP_SOFTWARE_INTERRUPT;
            trap->privilege = TRAP_MODE_MACHINE;
            break;
        case 5UL: //Supervisor timer interrupt
            trap->code = TRAP_TIMER_INTERRUPT;
            trap->privilege = TRAP_MODE_SUPERVISOR;
            break;
        case 7UL: //Machine timer interrupt
            trap->code = TRAP_TIMER_INTERRUPT;
            trap->privilege = TRAP_MODE_MACHINE;
            break;
        case 9UL: //Supervisor external interrupt
            trap->code = TRAP_EXTERNAL_INTERRUPT;
            trap->privilege = TRAP_MODE_SUPERVISOR;
            break;
        case 11UL: //Machine external interrupt
            trap->code = TRAP_EXTERNAL_INTERRUPT;
            trap->privilege = TRAP_MODE_MACHINE;
            break;
        case 13UL: //cnter-overflow interrupt
            //will code support for later when needed
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap->fault_addr, trap->fault_pc);
            break;
        default:
            PANIC("UNHANDLED_TRAP_INTERRUPT_OCCURRED",trap_code, trap->fault_addr, trap->fault_pc);
            break;
        }
    }else{
        trap->trap_type = TRAP_TYPE_EXCEPTION;
        switch (trap_code){
        case 0UL: //Instruction addr misaligned
            trap->code = TRAP_ACCESS_MISALIGNED;
            break;
        case 1UL: //Instruction access fault
            trap->code = TRAP_ACCESS_FAULT;
            break;
        case 2UL: //Illegal instruction
            trap->code = TRAP_INSTRUCTION_INVALID;
            break;
        case 3UL: //Breakpoint
            trap->code = TRAP_BREAKPOINT;
            break;
        case 4UL: //Load address misaligned
            trap->code = TRAP_ACCESS_MISALIGNED;
            break;
        case 5UL: //Load access fault
            trap->code = TRAP_ACCESS_FAULT;
            break;
        case 6UL: //Store/AMO address misaligned
            trap->code = TRAP_ACCESS_MISALIGNED;
            break;
        case 7UL: //Store/AMO access fault
            trap->code = TRAP_ACCESS_FAULT;
            break;
        case 8UL: //Environment call from U-mode
            trap->code = TRAP_SYSCALL;
            trap->privilege = TRAP_MODE_USER;
            break;
        case 9UL: //Environment call from S-mode
            trap->code = TRAP_SYSCALL;
            trap->privilege = TRAP_MODE_SUPERVISOR;
            break;
        case 11UL: //Environment call from M-mode
            trap->code = TRAP_SYSCALL;
            trap->privilege = TRAP_MODE_MACHINE;
            break;
        case 12UL: //Instruction page fault
            trap->code = TRAP_PAGE_FAULT;
            break;
        case 13UL: //Load page fault
            trap->code = TRAP_PAGE_FAULT;
            break;
        case 15UL: //Store/AMO page fault
            trap->code = TRAP_PAGE_FAULT;
            break;
        case 16UL: //Double trap
            trap->code = TRAP_DOUBLE_TRAP;
            break;
        case 18UL: //Software check
            trap->code = TRAP_SOFTWARE_CHECK;
            break;
        case 19UL: //Hardware error
            trap->code = TRAP_HARDWARE_ERROR;
            break;
        default:
            PANIC("UNHANDLED_TRAP_EXCEPTION_OCCURRED",trap_code, trap->fault_addr, trap->fault_pc);
            break;
        }
    }
}

void trap_data_set_response(trap_data *kernel_trap) {
    trapframe *trap_frame_data = (trapframe *)TRAPFRAME_ADDRESS;

    trap_frame_data->register_10 = kernel_trap->return_reg;
}

void trap_data_iter_instruction(trap_data *kernel_trap) {
    trapframe *trap_frame_data = (trapframe *)TRAPFRAME_ADDRESS;
    trap_frame_data->sepc += 4;
}

u64 trapframe_stack_ptr(trapframe *trap_frame) {
    return trap_frame->register_2;
}