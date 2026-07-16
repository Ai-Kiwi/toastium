#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

typedef enum {
    IRQ_UART,
    IRQ_TIMER,
    IRQ_SOFTWARE,
    IRQ_EXTERNAL
} IRQ_TYPES;

#endif