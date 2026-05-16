
#ifndef UART_16550_H
#define UART_16550_H

#include "include/types.h"

void uart_16550_output_u8(const u8 output_u8);
s32 uart_16550_try_fetch_u8();
void uart_16550_init();

#endif