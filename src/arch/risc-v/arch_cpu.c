#include "open_sbi.h"
#include "include/types.h"
#include "arch_cpu.h"

void freeze_system() {
    asm volatile ("csrc sstatus, %0" :: "r"(1 << 1)); //disable traps
    while (1){
        asm volatile("wfi");
    }
}

u64 cycle_cnt() {
    u64 cycle;
    asm volatile ("csrr %0, cycle" : "=r"(cycle));
    return cycle;
}

u64 time_cnt() {
    u64 time;
    asm volatile ("csrr %0, time" : "=r"(time));
    return time;
}

u64 timer_set(u64 cycle_num) {
    return open_sbi_set_timer(cycle_num);
}