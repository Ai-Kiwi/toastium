#include "kernel/trap/handler.h"
#include "drivers/uart/uart.h"
#include "arch_trap/parser.h"
#include "types.h"

//response of 0 means it handled it, higher means will need async version to handle it.
u64 kernel_syscall_sync_handler(kernel_trap_data *trap) {
    if (TRUE) {
        uart_print_char(trap->arg0_reg);
        arch_trap_iter_instruction(trap);
        return 0;
    }



    //actually very performant to return it must be async if it wasn't anything we handled
    //as it will likely be killed if it isn't async as well as then it would be unknown
    return 1;
}
