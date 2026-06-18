
#ifndef ARCH_HANDLER_H
#define ARCH_HANDLER_H

#include "types.h"
#include "arch_trap/parser.h"

void arch_trapframe_init_user(arch_trapframe *trapframe, u64 start_location);

#endif