#include "./../../kernel/irq.h"

void arch_enable_irq(IRQ_TYPES irq_type, int priority) {
    *(volatile int*)(0x0C002000) |= (1U << 10); //set interrupt for uart
    *(volatile int*)(0x0C001000 + 10 * 4) = 1; //set lowest priority for uart    
}
