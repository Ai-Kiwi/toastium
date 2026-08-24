#include "kernel/trap/handler.h"
#include "arch_trap/irq.h"
#include "def.h"
#include "drivers/uart/uart.h"
#include "handler.h"
#include "kernel/file_system/dentry.h"
#include "kernel/file_system/file_descriptor.h"
#include "kernel/memory/allocator.h"
#include "kernel/process/process.h"
#include "kernel/trap/user_accses.h"
#include "types.h"

u64 syscall_uart(trap_data *trap, bool8 async) {
    switch (trap->arg1_reg) {
    case 0: // char
        uart_print_char(trap->arg2_reg);
        return 0;
        break;
    case 1:
        const u64 str_src = trap->arg2_reg;
        const u64 str_size = trap->arg3_reg;
        if (async == FALSE) {
            return U64_MAX;
        }
        if (str_size > 512 || str_size == 0) {
            trap->return_reg = 1;
            return 0;
        }
        irq_disable();
        u8 *location = (u8 *)mem_alloc(str_size);
        irq_enable();
        bool8 response = kernel_read_user(str_src, str_size, (u64)location,
                                          (process *)trap->process_ptr);
        if (response == TRUE) {
            for (u64 i = 0; i < str_size; i++) {
                uart_print_char(location[i]);
            }
        }
        trap->return_reg = !response;
        irq_disable();
        mem_free((u64)location);
        irq_enable();

        // temp code to print from file

        // need releasing too
        file_descriptor *file =
            file_open_path(root_dentry_folder, "/example.txt");

        file_descriptor_seek(file, 50);

        char write_data[] = "[INSERT]";

        file_descriptor_write(file, write_data, 8);

        uart_println_str("now printing char");

        file_descriptor_seek(file, 0);

        u8 data_buf[128];
        u64 read_size = file_descriptor_read(file, (u8 *)data_buf, 128);

        for (u64 i = 0; i < 128; i++) {
            uart_print_char(data_buf[i]);
        }

        release_file_descriptor(file);

        return 0;
    default:
        return 1; // bad syscall
        break;
    }
}