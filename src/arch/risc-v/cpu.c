#include "open_sbi.h"

void arch_freeze_system() {
    asm volatile ("csrc sstatus, %0" :: "r"(1 << 1)); //disable traps
    while (1){
        asm volatile("wfi");
    }
}

long arch_get_cycle_count() {
    long cycle;
    asm volatile ("csrr %0, cycle" : "=r"(cycle));
    return cycle;
}

long arch_get_time_count() {
    long time;
    asm volatile ("csrr %0, time" : "=r"(time));
    return time;
}

long arch_set_timer(long cycle_number) {
    return open_sbi_set_timer(cycle_number);
}