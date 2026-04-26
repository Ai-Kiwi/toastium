
#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#include "kernel/trap/irq.h"

void arch_enable_irq(IRQ_TYPES irq_type);
void arch_irq_init();
void arch_irq_start();

#endif