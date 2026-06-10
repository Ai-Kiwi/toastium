#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include "types.h"

void kernel_set_timer(u64 clock_num);
void kernel_set_timer_future_ms(u64 ms);

#endif