#include "arch_irq.h"
#include "irq.h"

void irq_enable(IRQ_TYPES irq_type, int priority) {
    arch_enable_irq(irq_type, priority);
}

void irq_init() {
    arch_irq_init();
}