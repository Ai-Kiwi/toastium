
#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#include "./../../kernel/irq.h"

void arch_enable_irq(IRQ_TYPES irq_type, int priority);
void arch_irq_init();

#endif