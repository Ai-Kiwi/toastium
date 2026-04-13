#include "kernel/trap/irq.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "def.h"

//these values are S mode
#define PLIC_BASE 0x0C000000
#define PLIC_interrupt_toggles (PLIC_BASE 0x2000)
#define PLIC_threshold (PLIC_BASE + 0x201000) 





void arch_enable_irq(IRQ_TYPES irq_type) { //technically some of these are plic and interrupts so some shouldn't be here but they are merged, might unmerge later but would likely make more messy. 
    switch (irq_type){
    case KIRQ_UART:
        *(volatile int*)(PLIC_BASE + 10 * 4) = 1; //set lowest priority for uart
        *(volatile int*)(PLIC_BASE + 0x2080) |= (1U << 10); //set interrupt for uart
        break;
    case KIRQ_TIMER:
        asm volatile("csrs sie, %0" :: "r"BIT(5));
        break;
    case KIRQ_SOFTWARE:
        asm volatile("csrs sie, %0" :: "r"BIT(1));
        break;
    case KIRQ_EXTERNAL:
        asm volatile("csrs sie, %0" :: "r"BIT(9));
        break;
    default:
        PANIC("ATTEMPT_TO_ENABLE_UNHANDLED_IRQ",irq_type, 0, 0);
        break;
    }
}

void arch_irq_init() {
    asm volatile("csrs sstatus, %0" :: "r"BIT(1)); //enable global interrupts (bit 3 on)
    *(volatile int*)(PLIC_threshold) = 0; //set threshold to trigger trap
}