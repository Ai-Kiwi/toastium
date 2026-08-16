#include "kernel/trap/handler.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "handler.h"
#include "kernel/process/process.h"
#include "kernel/syscall/handler.h"
#include "kernel/syscall/uart/handler.h"
#include "types.h"

// response of 0 means it handled it
// response of 1 means process died
// higher means will need async version to handle it.
u64 syscall_sync_handler(trap_data *trap) {

    switch (trap->arg0_reg) {
    case SYSCALL_UART_PRINT:
        return syscall_uart(trap, FALSE);
        break;
    default:
        // will return async
        break;
    }

    // actually very performant to return it must be async if it wasn't anything
    // we handled as it will likely be killed if it isn't async as well as then
    // it would be unknown
    return U64_MAX;
}

// response 1 means process killed
u64 syscall_async_handler(trap_data *trap) {
    switch (trap->arg0_reg) {
    case SYSCALL_UART_PRINT:
        return syscall_uart(trap, TRUE);
        break;
    default:
        uart_println_str("process error: unknown syscall ");
        uart_print_str("process ID: ");
        uart_println_u64(((process *)trap->process_ptr)->process_id);
        uart_print_str("arg0: ");
        uart_println_u64(trap->arg0_reg);
        uart_print_str("arg1: ");
        uart_println_u64(trap->arg1_reg);
        uart_print_str("arg2: ");
        uart_println_u64(trap->arg2_reg);
        uart_print_str("arg3: ");
        uart_println_u64(trap->arg3_reg);
        return 1;
        break;
    }
}
