
#ifndef UART_H
#define UART_H

void uart_print_str(const char *print_string);
void uart_print_char(const char print_char);
void uart_init();


//utils, will be moved to debug service when needed as this becomes real driver
void uart_println_str(const char *print_string);
void uart_println_char(const char print_char) ;

void uart_print_ulong_hex(const unsigned long hex_value);
void uart_println_ulong_hex(const unsigned long hex_value);

void uart_print_uint_hex(const unsigned int hex_value);
void uart_println_uint_hex(const unsigned int hex_value);

void uart_print_char_hex(const unsigned long hex_value);
void uart_println_char_hex(const unsigned long hex_value);

void uart_print_ulong(unsigned long number);
void uart_println_ulong(unsigned long number);

void uart_print_long(long number);
void uart_println_long(long number);

#endif