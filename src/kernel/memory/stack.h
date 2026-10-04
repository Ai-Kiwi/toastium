#ifndef KERNEL_STACK_H
#define KERNEL_STACK_H

#include "types.h"

extern u64 *hart_stacks_list;

void stack_init(u64 hart_cnt);

#endif