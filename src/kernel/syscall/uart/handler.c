#include "kernel/trap/handler.h"
#include "arch_trap/irq.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "kernel/memory/allocator.h"
#include "types.h"
#include "kernel/trap/user_accses.h"


u64 syscall_uart(kernel_trap_data *trap, bool8 async) {
    switch (trap->arg1_reg) {
        case 0: //char
            uart_print_char(trap->arg2_reg);
            return 0;
            break;
        case 1:
            const u64 str_src = trap->arg2_reg;
            const u64 str_size = trap->arg3_reg;
            if (async == FALSE) {return U64_MAX;}
            if (str_size > 512 || str_size == 0) {trap->return_reg = 1; return 0;}
            arch_irq_disable();
            u8 *location = (u8 *)kernel_allocator_acquire(str_size);
            arch_irq_enable();
            bool8 response = kernel_read_user(str_src, str_size, (u64)location);
            if (response == TRUE) {
                for (u64 i = 0; i < str_size; i++) {
                    uart_print_char(location[i]);
                }
            }
            trap->return_reg = !response;
            arch_irq_disable();
            kernel_allocator_release((u64)location);
            arch_irq_enable();
            return 0;
        default:
            return 1;//bad syscall
            break;
    }
}