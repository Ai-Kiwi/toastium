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

typedef struct {
    u64 scause;
    u64 sepc;
    u64 stval;
    u64 sstatus;
} raw_trap_info;


void handle_unsafe_trap(raw_trap_info *trap) {//will have ptr input here that points to reg data on stack
    const u64 is_interrupt = trap->scause & BIT(63); //last bit
    const u64 trap_code = (trap->scause) & 0x7FFFFFFFFFFFFFFF; //everything but last bit
    const u64 is_user_mode = trap->scause & BIT(8);

    const u64 fault_addr = trap->stval;
    const u64 fault_pc = trap->sepc;

    if (is_interrupt) {
        switch (trap_code){
        case 1UL: //Supervisor software interrupt
            PANIC("PRE_BOOT_TRAP_SUPERVISOR_SOFTWARE_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 3UL: //Machine software interrupt
            PANIC("PRE_BOOT_TRAP_MACHINE_SOFTWARE_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 5UL: //Supervisor timer interrupt
            PANIC("PRE_BOOT_TRAP_SUPERVISOR_TIMER_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 7UL: //Machine timer interrupt
            PANIC("PRE_BOOT_TRAP_MACHINE_TIMER_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 9UL: //Supervisor external interrupt
            PANIC("PRE_BOOT_TRAP_SUPERVISOR_EXTERNAL_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 11UL: //Machine external interrupt
            PANIC("PRE_BOOT_TRAP_MACHINE_EXTERNAL_INTERRUPT", trap_code, fault_addr, fault_pc);
            break;
        case 13UL: //counter-overflow interrupt
            //will code support for later when needed
            PANIC("PRE_BOOT_TRAP_COUNTER_OVERFLOW", trap_code, fault_addr, fault_pc);
            break;
        default:
            PANIC("PRE_BOOT_TRAP_INTERRUPT_UNHANDED", trap_code, fault_addr, fault_pc);
            break;
        }
    }else{
        switch (trap_code){
        case 0UL: //Instruction addr misaligned
            PANIC("PRE_BOOT_TRAP_INSTRUCTION_ADDRESS_MISALIGNED", trap_code, fault_addr, fault_pc);
            break;
        case 1UL: //Instruction access fault
            PANIC("PRE_BOOT_TRAP_INSTRUCTION_ACCESS_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 2UL: //Illegal instruction
            PANIC("PRE_BOOT_TRAP_INSTRUCTION_INVALID", trap_code, fault_addr, fault_pc);
            break;
        case 3UL: //Breakpoint
            PANIC("PRE_BOOT_TRAP_BREAKPOINT", trap_code, fault_addr, fault_pc);
            break;
        case 4UL: //Load address misaligned
            PANIC("PRE_BOOT_TRAP_LOAD_ADDRESS_MISALIGNED_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 5UL: //Load access fault
            PANIC("PRE_BOOT_TRAP_LOAD_ACCESS_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 6UL: //Store/AMO address misaligned
            PANIC("PRE_BOOT_TRAP_STORE_MISALIGNED_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 7UL: //Store/AMO access fault
           PANIC("PRE_BOOT_TRAP_STORE_ACCESS_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 8UL: //Environment call from U-mode
            PANIC("PRE_BOOT_TRAP_USER_ENVIRONMENT_CALL", trap_code, fault_addr, fault_pc);
            break;
        case 9UL: //Environment call from S-mode
            PANIC("PRE_BOOT_TRAP_SUPERVISOR_ENVIRONMENT_CALL", trap_code, fault_addr, fault_pc);
            break;
        case 11UL: //Environment call from M-mode
            PANIC("PRE_BOOT_TRAP_MACHINE_ENVIRONMENT_CALL", trap_code, fault_addr, fault_pc);
            break;
        case 12UL: //Instruction page fault
            PANIC("PRE_BOOT_TRAP_INSTRUCTION_PAGE_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 13UL: //Load page fault
            PANIC("PRE_BOOT_TRAP_LOAD_PAGE_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 15UL: //Store/AMO page fault
            PANIC("PRE_BOOT_TRAP_STORE_PAGE_FAULT", trap_code, fault_addr, fault_pc);
            break;
        case 16UL: //Double trap
            PANIC("PRE_BOOT_TRAP_DOUBLE_TRAP", trap_code, fault_addr, fault_pc);
            break;
        case 18UL: //Software check
            PANIC("PRE_BOOT_TRAP_SOFTWARE_CHECK", trap_code, fault_addr, fault_pc);
            break;
        case 19UL: //Hardware error
            PANIC("PRE_BOOT_TRAP_HARDWARE_ERROR", trap_code, fault_addr, fault_pc);
            break;
        default:
            PANIC("PRE_BOOT_TRAP_EXCEPTION_UNHANDED", trap_code, fault_addr, fault_pc);
            break;
        }
    }

    PANIC("PRE_BOOT_TRAP_UNHANDED", trap_code, fault_addr, fault_pc);
}