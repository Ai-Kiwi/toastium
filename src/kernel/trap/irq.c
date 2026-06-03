#include "arch_trap/irq.h"
#include "irq.h"

void irq_enable(IRQ_TYPES irq_type) {
    arch_enable_irq(irq_type);
}

void irq_init() {
    arch_irq_init();
}