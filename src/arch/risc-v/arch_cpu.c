#include "open_sbi.h"
#include "include/types.h"

void arch_freeze_system() {
    asm volatile ("csrc sstatus, %0" :: "r"(1 << 1)); //disable traps
    while (1){
        asm volatile("wfi");
    }
}

u64 arch_get_cycle_count() {
    u64 cycle;
    asm volatile ("csrr %0, cycle" : "=r"(cycle));
    return cycle;
}

u64 arch_get_time_count() {
    u64 time;
    asm volatile ("csrr %0, time" : "=r"(time));
    return time;
}

u64 arch_set_timer(u64 cycle_number) {
    return open_sbi_set_timer(cycle_number);
}