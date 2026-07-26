#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/safety/safety.h"
#include "kernel/process/scheduler.h"
#include "kernel/trap/handler.h"
#include "arch_trap/parser.h"
#include "board.h"
#include "arch_vma/virtual_memory.h"
#include "drivers/uart/uart.h"
#include "kernel/timer/timer.h"
#include "kernel/syscall/handler.h"
#include "kernel/process/context.h"
#include "types.h"

//function returns 0 if it was handled in kernel.
//function returns process kernel stack if its to be handled using process stack/kernel

//even if process is changed asm doesn't need to worry (vma swap and trap_frame swap done in c)

void trap_change_process(trap_data *trap_data) {
    process *next_process = scheduler_next(trap_data->hart_id);
    context_change_process(next_process, trap_data->hart_id);

    timer_set_future_ms(4);
}

u64 handle_sync_trap() {
    trap_data trap;
    trapframe_parse((trap_data *)&trap);

    //decide to use
    process *proc = (process *)trap.process_ptr;
    const u64 kernel_process_stack = PROCESS_KERNEL_STACK_START + (PROCESS_KERNEL_STACK_SIZE - 1); //12KB

    switch (trap.code) {
    case TRAP_ACCESS_MISALIGNED:
        uart_println_str("process killed : access misaligned");
        uart_print_str("process id:");
        uart_println_u64(proc->process_id);
        uart_print_str("fault addr:");
        uart_println_u64(trap.fault_addr);
        uart_print_str("fault pc:");
        uart_println_u64(trap.fault_pc);

        kill_process(((process *)trap.process_ptr)->process_id);
        trap_change_process(&trap);
        return 0;
        break;
    case TRAP_ACCESS_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_FAULT",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_INSTRUCTION_INVALID:
        uart_println_str("process killed : bad instruction");
        uart_print_str("process id:");
        uart_println_u64(proc->process_id);
        uart_print_str("fault addr:");
        uart_println_u64(trap.fault_addr);
        uart_print_str("fault pc:");
        uart_println_u64(trap.fault_pc);

        kill_process(((process *)trap.process_ptr)->process_id);
        trap_change_process(&trap);
        return 0;
        break;
    case TRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_SYSCALL:
        u64 response = syscall_sync_handler(&trap);
        if (response == 1) {//process needs to be killed
            kill_process(((process *)trap.process_ptr)->process_id);

            trap_change_process(&trap);
            return 0;
        }
        if (response > 0) {
            //needs to be handled by async
            return kernel_process_stack;
        }
        trap_data_set_response(&trap);
        trap_data_iter_instruction(&trap);
        break;
    case TRAP_PAGE_FAULT:
        if (proc->trap_state == PROC_TRAP_PROCESS_TRAP) {
            u64 stack_location = trapframe_stack_ptr(proc->kernelspace_trapframe);
            stack_location = ROUND_MOD_DOWN(stack_location - 8, 8);
            return stack_location;
        }else{
            return kernel_process_stack;
        }
        break;
    case TRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    case TRAP_TIMER_INTERRUPT:
        if (trap.privilege != TRAP_MODE_SUPERVISOR){
            PANIC("KERNEL_TRAP_TIMER_NON_SUPERVISOR_PRIVILEGE",trap.privilege, trap.fault_addr, trap.fault_pc);
        }

        trap_change_process(&trap);

        break;
    case TRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED",trap.privilege, trap.fault_addr, trap.fault_pc);
        break;
    }

    return 0; //was kernel stack handled
}

//Will return trap action
//Take action first, like kill program or what not. Or other things like add disk write to queue
//Return response, decide if program immediately resumes or schedular is run

//if program runs or resumes it depends on trap that was called.
//For some calls like syscalls with for example disk write, there will be non block and block operations, this decides if to return or reschedule