#include "cpu.h"
#include "uart/uart.h"
#include "panic.h"

void uart_print_hex(const long hex_value) {
    static const char hex_list[] = "0123456789ABCDEF";
    for (int i=15; i > -1; i=i-1) { //64/4=16
        int digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(&hex_list[digit_value]);
    }
}

void kernel_panic(const char *file, const long file_line, const char *function, const char *message, long extra_value) {
    uart_print_chars("\n _  ________ _____  _   _ ______ _        _____        _   _ _____ _____ \n");
    uart_print_chars("| |/ /  ____|  __ \\| \\ | |  ____| |      |  __ \\ /\\   | \\ | |_   _/ ____|\n");
    uart_print_chars("| ' /| |__  | |__) |  \\| | |__  | |      | |__) /  \\  |  \\| | | || |     \n");
    uart_print_chars("|  < |  __| |  _  /| . ` |  __| | |      |  ___/ /\\ \\ | . ` | | || |     \n");
    uart_print_chars("| . \\| |____| | \\ \\| |\\  | |____| |____  | |  / ____ \\| |\\  |_| || |____ \n");
    uart_print_chars("|_|\\_\\______|_|  \\_\\_| \\_|______|______| |_| /_/    \\_\\_| \\_|_____\\_____|\n");

    uart_print_chars("\nKERNEL PANIC\n");
    uart_print_chars("The experts call this not good and I call this not good.\n\n");
    uart_print_chars("Here is some info relating to what went wrong...\n");
    uart_print_chars("-------------------------------------------------------------------------\n");
    
    //file, function and line
    uart_print_chars("LOCATION : ");
    uart_print_chars(file);
    uart_print_chars(":");
    uart_print_chars(function);
    uart_print_chars(" (0x");
    uart_print_hex(file_line);
    uart_print_chars(")\n");

    //reason
    uart_print_chars("REASON   : ");
    uart_print_chars(message);

    //print out hex code for number
    uart_print_chars("\nVALUE    : 0x");
    uart_print_hex(extra_value);
    uart_print_chars("\n");

    arch_freeze_system();
}