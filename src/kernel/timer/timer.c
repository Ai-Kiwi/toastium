//for now timer just stops program running and calls schedular.
//later will have a system that has list of timers and looks at most recent and uses that.

#define BASE_CLOCK_SPEED 10000000 //clocks per second

#include "arch_cpu.h"
#include "kernel/safety/panic.h"

void timer_set(u64 clock_num) {
    u64 response = arch_set_timer(clock_num);
    if (response) {
        PANIC("FAILED_TO_CREATE_KERNEL_TIMER", response, 0, 0);
    }
}

void kernel_timer_set_future_ms(u64 ms) {
    u64 response = arch_set_timer(arch_get_time_cnt() + ((BASE_CLOCK_SPEED / 1000) * ms));
    if (response) {
        PANIC("FAILED_TO_CREATE_KERNEL_TIMER", response, 0, 0);
    }
}

//future outline
//
//void kernel_timer_schedule(kernel_timer *info, future_ms) {
//    //add a timer to the list
//}
//
//void kernel_timer_return_finished(kernel_timer *returned) {
//    //finish the next timer on the list. Will be one that just set off, also removes it.
//}
//
//void kernel_timer_resume() {
//    //Sets for the next timer to go off.
//}