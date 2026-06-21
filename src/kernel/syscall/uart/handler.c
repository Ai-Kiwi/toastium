#include "kernel/trap/handler.h"
#include "drivers/uart/uart.h"
#include "types.h"


u64 syscall_uart(kernel_trap_data *trap) {
    switch (trap->arg1_reg) {
        case 0: //char
            uart_print_char(trap->arg2_reg);
            return 0;
            break;
        case 1:

        default:
            return 1;//bad syscall
            break;
    }
}