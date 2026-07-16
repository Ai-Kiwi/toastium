
#ifndef ARCH_IRQ_H
#define ARCH_IRQ_H

#include "kernel/trap/irq.h"

void irq_enable_type(IRQ_TYPES irq_type);
void irq_init();
void irq_enable();
void irq_disable();

#endif