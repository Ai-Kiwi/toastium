#ifndef IRQ_H
#define IRQ_H

typedef enum {
    KIRQ_UART,
    KIRQ_TIMER,
    KIRQ_SOFTWARE,
    KIRQ_EXTERNAL
} IRQ_TYPES;

#endif