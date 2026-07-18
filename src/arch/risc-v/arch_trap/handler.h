
#ifndef ARCH_HANDLER_H
#define ARCH_HANDLER_H

#include "types.h"
#include "arch_trap/parser.h"

void trapframe_user_init(trapframe *trapframe, u64 start_location);

#endif