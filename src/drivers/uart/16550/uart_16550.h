
#ifndef UART_16550_H
#define UART_16550_H

void uart_16550_output_char(const char output_char);
int uart_16550_try_fetch_char();
void uart_16550_init();

#endif