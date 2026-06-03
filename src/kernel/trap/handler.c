#include "kernel/trap/handler.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"
#include "kernel/process/scheduler.h"
#include "kernel/trap/handler.h"

//function returns 0 if it was handled in kernel.
//function returns process kernel stack if its to be handled using process stack/kernel

//even if process is changed asm doesn't need to worry (vma swap and trap_frame swap done in c)

u64 kernel_handle_trap() {
    kernel_trap_data trap_data;
    arch_parse_trap_data(trap_data);

    //decide to use

    switch (trap_data.code) {
    case KTRAP_ACCESS_MISALIGNED:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_MISALIGNED",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_ACCESS_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_FAULT",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_INSTRUCTION_INVALID:
        PANIC("KERNEL_TRAP_UNIMPLENTED_INSTRUCTION_INVALID",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_SYSCALL:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SYSCALL",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_PAGE_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_PAGE_FAULT",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    case KTRAP_TIMER_INTERRUPT:
        if (trap_data.privilege != KTRAP_MODE_SUPERVISOR){
            PANIC("KERNEL_TRAP_TIMER_NON_SUPERVISOR_PRIVILEGE",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        }
        break;
    case KTRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED",trap_data.privilege, trap_data.fault_address, trap_data.fault_pc);
        break;
    }

    return 0; //was kernel stack handled
}

//Will return trap action
//Take action first, like kill program or what not. Or other things like add disk write to queue
//Return response, decide if program immediately resumes or schedular is run

//if program runs or resumes it depends on trap that was called.
//For some calls like syscalls with for example disk write, there will be non block and block operations, this decides if to return or reschedule