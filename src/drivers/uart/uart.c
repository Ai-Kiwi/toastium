//This layout if so that if there is more then 1 uart chip it would have a folder for each chip under one uart folder, then that uart folder would have a interface which translates calls to whichever chip in question is needed
#include "uart.h"
#include "16550/uart_16550.h"
#include "include/types.h"

//strings
void uart_print_char(const u8 print_char) {
    uart_16550_output_u8(print_char);
    if (print_char == '\n') {
        uart_16550_output_u8('\r');
    }
}

void uart_print_str(const u8 *print_string) {
    for (u8 *p = print_string; *p != '\0' ; p++) {
        uart_16550_output_u8(*p);
    }
}

void uart_init() {
    uart_16550_init();
}


//All utils below built off this man one.
void uart_println_char(const u8 print_char) {
    uart_print_char(print_char);
    uart_print_char('\n');
}

void uart_println_str(const u8 *print_string) {
    uart_print_str(print_string);
    uart_print_char('\n');
}

//hex
void uart_print_u64_hex(const u64 hex_value) {
    static const u8 hex_list[] = "0123456789ABCDEF";
    for (s32 i=15; i > -1; i=i-1) {
        s32 digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_u64_hex(const u64 hex_value) {
    uart_print_u64_hex(hex_value);
    uart_print_char('\n');
}

void uart_print_u32_hex(const u32 hex_value) {
    static const u8 hex_list[] = "0123456789ABCDEF";
    for (s32 i=7; i > -1; i=i-1) {
        s32 digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_u32_hex(const u32 hex_value) {
    uart_print_u32_hex(hex_value);
    uart_print_char('\n');
}

void uart_print_u8_hex(const u8 hex_value) {
    static const u8 hex_list[] = "0123456789ABCDEF";
    for (s32 i=1; i > -1; i=i-1) {
        s32 digit_value = (hex_value >> (i * 4)) & 0xF;
        uart_print_char(hex_list[digit_value]);
    }
}

void uart_println_u8_hex(const u8 hex_value) {
    uart_print_u8_hex(hex_value);
    uart_print_char('\n');
}

//nums
void uart_print_u64(u64 num) {
    u8 digits[20] = {};
    s32 bottom = 0;
    for (s32 i=19; i > -1; i=i-1) {
        u8 digit = num % 10;
        digits[i] = digit;
        num /= 10;
        if (num == 0) {
            bottom = i;
            break;
        }
    }
    static const u8 hex_list[] = "0123456789";
    for (s32 i=bottom; i < 20; i++) {
        uart_print_char(hex_list[digits[i]]);
    }
}

void uart_println_u64(u64 num) {
    uart_print_u64(num);
    uart_print_char('\n');
}

void uart_print_s64(s64 num) {
    if (num < 0) {
        uart_print_char('-');
        num = -num;
    }

    u8 digits[20] = {};
    s32 bottom = 0;
    for (s32 i=19; i > -1; i=i-1) {
        u8 digit = num % 10;
        digits[i] = digit;
        num /= 10;
        if (num == 0) {
            bottom = i;
            break;
        }
    }
    static const u8 hex_list[] = "0123456789";
    for (s32 i=bottom; i < 20; i++) {
        uart_print_char(hex_list[digits[i]]);
    }
}

void uart_println_s64(s64 num) {
    uart_print_s64(num);
    uart_print_char('\n');
}

