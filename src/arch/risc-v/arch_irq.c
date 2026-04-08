#include "./../../kernel/irq.h"
#include "../../drivers/uart/uart.h"

//these values are S mode
#define PLIC_BASE 0x0C000000
#define PLIC_interrupt_toggles (PLIC_BASE 0x2000)
#define PLIC_threshold (PLIC_BASE + 0x201000) 





void arch_enable_irq(IRQ_TYPES irq_type, int priority) {
    *(volatile int*)(PLIC_BASE + 10 * 4) = 1; //set lowest priority for uart
    *(volatile int*)(PLIC_BASE + 0x2080) |= (1U << 10); //set interrupt for uart

    *(volatile int*)(PLIC_threshold) = 0;
}

void arch_irq_init() {
    asm volatile("csrs sie, %0" :: "r"(1 << 9)); //enable external traps (bit 10 on)
    asm volatile("csrs sstatus, %0" :: "r"(1 << 1)); //enable global interrupts (bit 3 on)
}