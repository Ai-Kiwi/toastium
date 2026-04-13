#include "kernel/trap/handler.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"
#include "kernel/process/scheduler.h"

const kernel_trap_response interrupt_trap(const kernel_trap_data *trap_data) {
    kernel_trap_response trap_response;
    switch (trap_data->code){
    case KTRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_TIMER_INTERRUPT:
        if (trap_data->privilege != KTRAP_MODE_SUPERVISOR){
            PANIC("KERNEL_TRAP_TIMER_NON_SUPERVISOR_PRIVILEGE",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        }
        trap_response.kernel_PID = kernel_scheduler_next_process();
        trap_response.response_type = KTRAP_RESPONSE_HOLD_PROCESS;
        break;
    case KTRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED_INTERRUPT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    }
    return trap_response;
}

const kernel_trap_response exception_trap(const kernel_trap_data *trap_data) {
    kernel_trap_response trap_response;
    switch (trap_data->code) {
    case KTRAP_ACCESS_MISALIGNED:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_MISALIGNED",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_ACCESS_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_FAULT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_INSTRUCTION_INVALID:
        PANIC("KERNEL_TRAP_UNIMPLENTED_INSTRUCTION_INVALID",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_SYSCALL:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SYSCALL",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_PAGE_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_PAGE_FAULT",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    case KTRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc); 
        break;
    case KTRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED_EXCEPTION",trap_data->privilege, trap_data->fault_address, trap_data->fault_pc);
        break;
    }
    return trap_response;
}

const kernel_trap_response kernel_handle_trap(const kernel_trap_data *trap_data) {
    kernel_safety_test();

    switch (trap_data->trap_type){
    case KTRAP_TYPE_EXCEPTION:
        return exception_trap(trap_data);
        break;
    case KTRAP_TYPE_INTERRUPT:
        return interrupt_trap(trap_data);
        break;
    default:
        PANIC("KERNEL_TRAP_UNKOWN_TYPE",trap_data->trap_type,trap_data->code,trap_data->fault_pc);
        break;
    }

}

//Will return trap action
//Take action first, like kill program or what not. Or other things like add disk write to queue
//Return response, decide if program immediately resumes or schedular is run

//if program runs or resumes it depends on trap that was called. 
//For some calls like syscalls with for example disk write, there will be non block and block operations, this decides if to return or reschedule