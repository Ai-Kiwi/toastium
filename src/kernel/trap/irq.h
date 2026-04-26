#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

typedef enum {
    KIRQ_UART,
    KIRQ_TIMER,
    KIRQ_SOFTWARE,
    KIRQ_EXTERNAL
} IRQ_TYPES;

void irq_enable(IRQ_TYPES irq_type);
void irq_init();
void irq_start();

#endif