//This layout if so that if there is more then 1 uart chip it would have a folder for each chip under one uart folder, then that uart folder would have a interface which translates calls to whichever chip in question is needed
#include "uart.h"
#include "16550/uart_16550.h"

//strings
void uart_print_char(const char print_char) {
    uart_16550_output_char(print_char);
}

void uart_print_str(const char *print_string) {
    for (char *p = print_string; *p != '\0' ; p++) {
        uart_16550_output_char(*p);
    }
}

void uart_init() {
    uart_16550_init();
}


//All utils below built off this man one.
void uart_println_char(const char print_char) {
    uart_print_char(print_char);
    uart_print_char('\n');
}

void uart_println_str(const char *print_string) {
    uart_print_str(print_string);
    uart_print_char('\n');
}

//hex
void uart_print_ulong_hex(const unsigned long hex_value) {
    static const char hex_list[] = "0123456789ABCDEF";
    for (int i=15; i > -1; i=i-1) {
        int digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_ulong_hex(const unsigned long hex_value) {
    uart_print_ulong_hex(hex_value);
    uart_print_char('\n');
}

void uart_print_uint_hex(const unsigned int hex_value) {
    static const char hex_list[] = "0123456789ABCDEF";
    for (int i=7; i > -1; i=i-1) {
        int digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_uint_hex(const unsigned int hex_value) {
    uart_print_uint_hex(hex_value);
    uart_print_char('\n');
}

void uart_print_char_hex(const unsigned long hex_value) {
    static const char hex_list[] = "0123456789ABCDEF";
    for (int i=1; i > -1; i=i-1) {
        int digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_char_hex(const unsigned long hex_value) {
    uart_print_char_hex(hex_value);
    uart_print_char('\n');
}

//numbers
void uart_print_ulong(unsigned long number) {
    char digits[20] = {};
    int bottom = 0;
    for (int i=19; i > -1; i=i-1) {
        char digit = number % 10;
        digits[i] = digit;
        number /= 10;
        if (number == 0) {
            bottom = i;
            break;
        }
    }
    static const char hex_list[] = "0123456789";
    for (int i=bottom; i < 20; i++) {
        uart_print_char(hex_list[digits[i]]);
    }
}

void uart_println_ulong(unsigned long number) {
    uart_print_ulong(number);
    uart_print_char('\n');
}

void uart_print_long(long number) {
    if (number < 0) {
        uart_print_char('-');
        number = -number;
    }

    char digits[20] = {};
    int bottom = 0;
    for (int i=19; i > -1; i=i-1) {
        char digit = number % 10;
        digits[i] = digit;
        number /= 10;
        if (number == 0) {
            bottom = i;
            break;
        }
    }
    static const char hex_list[] = "0123456789";
    for (int i=bottom; i < 20; i++) {
        uart_print_char(hex_list[digits[i]]);
    }
}

void uart_println_long(long number) {
    uart_print_long(number);
    uart_print_char('\n');
}

