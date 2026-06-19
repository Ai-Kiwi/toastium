#include "kernel/trap/handler.h"
#include "drivers/uart/uart.h"
#include "arch_trap/parser.h"

void kernel_syscall_sync_handler(kernel_trap_data *trap) {
    uart_print_char(trap->arg0_reg);

    arch_trap_iter_instruction(trap);
}
