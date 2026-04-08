#include "cpu.h"
#include "uart/uart.h"

void panic(char *message, long extra_value) {
    uart_print_chars("\n _  ________ _____  _   _ ______ _        _____        _   _ _____ _____ \n");
    uart_print_chars("| |/ /  ____|  __ \\| \\ | |  ____| |      |  __ \\ /\\   | \\ | |_   _/ ____|\n");
    uart_print_chars("| ' /| |__  | |__) |  \\| | |__  | |      | |__) /  \\  |  \\| | | || |     \n");
    uart_print_chars("|  < |  __| |  _  /| . ` |  __| | |      |  ___/ /\\ \\ | . ` | | || |     \n");
    uart_print_chars("| . \\| |____| | \\ \\| |\\  | |____| |____  | |  / ____ \\| |\\  |_| || |____ \n");
    uart_print_chars("|_|\\_\\______|_|  \\_\\_| \\_|______|______| |_| /_/    \\_\\_| \\_|_____\\_____|\n");

    uart_print_chars("\nKERNEL PANIC\n");
    uart_print_chars("the experts call this not good and I call this not good.\n\n");
    uart_print_chars("here is some info relating to what went wrong...\n");
    
    uart_print_chars("REASON : ");
    uart_print_chars(message);
    //print out hex code for number
    uart_print_chars("\nVALUE : 0x");
    char *hex_list = "0123456789ABCDEF";

    for (int i=15; i > -1; i=i-1) { //64/4=16
        int digit_value = (extra_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list + digit_value);
    }
    uart_print_chars("\n");

    freeze_system();
}