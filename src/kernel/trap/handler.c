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

//function returns 0 if it was handled in kernel.
//function returns process kernel stack if its to be handled using process stack/kernel

//even if process is changed asm doesn't need to worry (vma swap and trap_frame swap done in c)

void change_process(kernel_trap_data *trap_data) {
    kernel_process *next_process = kernel_scheduler_dequeue_next_process(trap_data->hart_id);
    kernel_context_change_process(next_process, trap_data->hart_id);

    kernel_timer_set_future_ms(4);
}

u64 kernel_handle_trap() {
    kernel_trap_data trap_data;
    arch_parse_trap_data((kernel_trap_data *)&trap_data);

    //decide to use
    kernel_process *process = (kernel_process *)trap_data.process_ptr;

    switch (trap_data.code) {
    case KTRAP_ACCESS_MISALIGNED:
        uart_println_str("process killed : access misaligned");
        uart_print_str("process id:");
        uart_println_u64(process->process_id);
        uart_print_str("fault addr:");
        uart_println_u64(trap_data.fault_addr);
        uart_print_str("fault pc:");
        uart_println_u64(trap_data.fault_pc);

        kernel_process_kill_process(((kernel_process *)trap_data.process_ptr)->process_id);
        change_process(&trap_data);
        return 0;
        break;
    case KTRAP_ACCESS_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_ACCESS_FAULT",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_INSTRUCTION_INVALID:
        uart_println_str("process killed : bad instruction");
        uart_print_str("process id:");
        uart_println_u64(process->process_id);
        uart_print_str("fault addr:");
        uart_println_u64(trap_data.fault_addr);
        uart_print_str("fault pc:");
        uart_println_u64(trap_data.fault_pc);

        kernel_process_kill_process(((kernel_process *)trap_data.process_ptr)->process_id);
        change_process(&trap_data);
        return 0;
        break;
    case KTRAP_BREAKPOINT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_BREAKPOINT",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_SYSCALL:
        u64 response = kernel_syscall_sync_handler(&trap_data);
        if (response == 1) {//process needs to be killed
            kernel_process_kill_process(((kernel_process *)trap_data.process_ptr)->process_id);

            change_process(&trap_data);
            return 0;
        }
        if (response > 0) {
            //needs to be handled by async
            return ((u64)((kernel_process *)trap_data.process_ptr)->kernel_stack) + KERNEL_PAGE_SIZE - 1;
        }
        arch_trap_set_response(&trap_data);
        arch_trap_iter_instruction(&trap_data);
        break;
    case KTRAP_PAGE_FAULT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_PAGE_FAULT",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_DOUBLE_TRAP:
        PANIC("KERNEL_TRAP_DOUBLE_TRAP",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_SOFTWARE_CHECK:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_CHECK",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_HARDWARE_ERROR:
        PANIC("KERNEL_TRAP_UNIMPLENTED_HARDWARE_ERROR",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_SOFTWARE_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_SOFTWARE_INTERRUPT",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    case KTRAP_TIMER_INTERRUPT:
        if (trap_data.privilege != KTRAP_MODE_SUPERVISOR){
            PANIC("KERNEL_TRAP_TIMER_NON_SUPERVISOR_PRIVILEGE",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        }

        change_process(&trap_data);

        break;
    case KTRAP_EXTERNAL_INTERRUPT:
        PANIC("KERNEL_TRAP_UNIMPLENTED_EXTERNAL_INTERRUPT",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    default:
        PANIC("KERNEL_TRAP_UNHANDLED",trap_data.privilege, trap_data.fault_addr, trap_data.fault_pc);
        break;
    }

    return 0; //was kernel stack handled
}

//Will return trap action
//Take action first, like kill program or what not. Or other things like add disk write to queue
//Return response, decide if program immediately resumes or schedular is run

//if program runs or resumes it depends on trap that was called.
//For some calls like syscalls with for example disk write, there will be non block and block operations, this decides if to return or reschedule