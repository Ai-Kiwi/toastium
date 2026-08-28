#include "kernel/trap/handler.h"
#include "arch_cpu.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "handler.h"
#include "kernel/process/process.h"
#include "kernel/syscall/file_system/handler.h"
#include "kernel/syscall/handler.h"
#include "kernel/syscall/uart/handler.h"
#include "types.h"

// response of 0 means it handled it
// response of 1 means it was unable hand it (invalid)
// for not response of 1 will kill process however later will give it error

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

u64 syscall_async_handler(trap_data *trap) {
    switch (trap->arg0_reg) {
    case SYSCALL_UART_PRINT:
        return syscall_uart(trap, TRUE);
        break;
    case SYSCALL_IDLE_HANG:
        idle_loop_system();
        break;
    case SYSCALL_FS_OPEN:
        syscall_fs_open(trap);
        break;
    case SYSCALL_FS_READ:
        syscall_fs_read(trap);
        break;
    case SYSCALL_FS_WRITE:
        syscall_fs_write(trap);
        break;
    case SYSCALL_FS_SEEK:
        syscall_fs_seek(trap);
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

    // a response must have handled it to end up here as default didn't so
    // return 0;
    return 0;
}
