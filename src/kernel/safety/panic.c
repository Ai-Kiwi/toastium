#include "arch_cpu.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "include/board.h"

void kernel_panic(const char *file, const long file_line, const char *function, const char *message, long extra_value_1, long extra_value_2, long extra_value_3) {
    uart_println_str("");
    uart_println_str(" _  ________ _____  _   _ ______ _        _____        _   _ _____ _____ ");
    uart_println_str("| |/ /  ____|  __ \\| \\ | |  ____| |      |  __ \\ /\\   | \\ | |_   _/ ____|");
    uart_println_str("| ' /| |__  | |__) |  \\| | |__  | |      | |__) /  \\  |  \\| | | || |     ");
    uart_println_str("|  < |  __| |  _  /| . ` |  __| | |      |  ___/ /\\ \\ | . ` | | || |     ");
    uart_println_str("| . \\| |____| | \\ \\| |\\  | |____| |____  | |  / ____ \\| |\\  |_| || |____ ");
    uart_println_str("|_|\\_\\______|_|  \\_\\_| \\_|______|______| |_| /_/    \\_\\_| \\_|_____\\_____|");

    uart_println_str("KERNEL PANIC");
    uart_println_str("The experts call this not good and I call this not good.");
    uart_println_str("");
    uart_println_str("Here is some info relating to what went wrong...");
    uart_println_str("-------------------------------------------------------------------------");

    //file, function and line
    uart_print_str("GIT VERSION HASH : ");
    uart_print_str(COMPILE_VERSION);
    uart_print_str(", COMPILE DATE : ");
    uart_println_str(__DATE__);

    //file, function and line
    uart_print_str("LOCATION : ");
    uart_print_str(file);
    uart_print_str(":");
    uart_print_long(file_line);
    uart_print_str(" (");
    uart_print_str(function);
    uart_println_str(")");

    //reason
    uart_print_str("REASON   : ");
    uart_println_str(message);

    //print out hex code for number
    uart_print_str("VALUE 1  : 0x");
    uart_print_ulong_hex(extra_value_1);
    uart_print_str(" - (");
    uart_print_long(extra_value_1);
    uart_println_str(")");

    uart_print_str("VALUE 2 : 0x");
    uart_print_ulong_hex(extra_value_2);
    uart_print_str(" - (");
    uart_print_long(extra_value_2);
    uart_println_str(")");

    uart_print_str("VALUE 3 : 0x");
    uart_print_ulong_hex(extra_value_3);
    uart_print_str(" - (");
    uart_print_long(extra_value_3);
    uart_println_str(")");

    arch_freeze_system();
}