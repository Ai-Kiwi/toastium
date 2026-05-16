//for now timer just stops program running and calls schedular.
//later will have a system that has list of timers and looks at most recent and uses that.

#define BASE_CLOCK_SPEED 10000000 //clocks per second

#include "arch_cpu.h"
#include "kernel/safety/panic.h"

void kernel_set_timer(u64 clock_number) {
    u64 response = arch_set_timer(clock_number);
    if (response) {
        PANIC("FAILED_TO_CREATE_KERNEL_TIMER", response, 0, 0);
    }
}

void kernel_set_timer_future_ms(u64 ms) {
    u64 response = arch_set_timer(arch_get_time_count() + ((BASE_CLOCK_SPEED / 1000) * ms));
    if (response) {
        PANIC("FAILED_TO_CREATE_KERNEL_TIMER", response, 0, 0);
    }
}