#ifndef UART_H
#define UART_H

#include "include/types.h"

void uart_print_str(const u8 *print_string);
void uart_print_char(const u8 print_char);
void uart_init();


//utils, will be moved to debug service when needed as this becomes real driver
void uart_println_str(const u8 *print_string);
void uart_println_char(const u8 print_char) ;

void uart_print_u64_hex(const u64 hex_value);
void uart_println_u64_hex(const u64 hex_value);

void uart_print_u32_hex(const u32 hex_value);
void uart_println_u32_hex(const u32 hex_value);

void uart_print_u8_hex(const u8 hex_value);
void uart_println_u8_hex(const u8 hex_value);

void uart_print_u64(u64 number);
void uart_println_u64(u64 number);

void uart_print_s64(s64 number);
void uart_println_s64(s64 number);

#endif