#include "kernel_trap.h"
#include "panic.h"
#include "safety.h"

void interrupt_trap(const kernel_trap_data *trap_data) {
    switch (trap_data->code){
    case KTRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT",trap_data->trap_type);
        break;
    case KTRAP_TIMER_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_TIMER_INTERRUPT",trap_data->trap_type);
        break;
    case KTRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT",trap_data->trap_type);
        break;
    default:
        break;
    }
}

void exception_trap(const kernel_trap_data *trap_data) {
    switch (trap_data->code) {
    case KTRAP_ACCESS_MISALIGNED:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_MISALIGNED",trap_data->trap_type);
        break;
    case KTRAP_ACCESS_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_FAULT",trap_data->trap_type);
        break;
    case KTRAP_INSTRUCTION_INVALID:
        PANIC("KERNEL_TRAP_UNIMPLENTED_INSTRUCTION_INVALID",trap_data->trap_type);
        break;
    case KTRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT",trap_data->trap_type);
        break;
    case KTRAP_SYSCALL:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SYSCALL",trap_data->trap_type);
        break;
    case KTRAP_PAGE_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_PAGE_FAULT",trap_data->trap_type);
        break;
    case KTRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP",trap_data->trap_type);
        break;
    case KTRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK",trap_data->trap_type);    
        break;
    case KTRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR",trap_data->trap_type);
        break;
    default:
        break;
    }
}

void kernel_handle_trap(const kernel_trap_data *trap_data) {
    kernel_safety_test();

    switch (trap_data->trap_type){
    case KTRAP_TYPE_EXCEPTION:
        exception_trap(trap_data);
        break;
    case KTRAP_TYPE_INTERRUPT:
        interrupt_trap(trap_data);
        break;
    default:
        PANIC("KERNEL_TRAP_UNKOWN_TYPE",trap_data->trap_type);
        break;
    }

}

//Will return trap action
//Take action first, like kill program or what not. Or other things like add disk write to queue
//Return response, decide if program immediately resumes or schedular is run

//if program runs or resumes it depends on trap that was called. 
//For some calls like syscalls with for example disk write, there will be non block and block operations, this decides if to return or reschedule