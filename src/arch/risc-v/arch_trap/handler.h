
#ifndef ARCH_HANDLER_H
#define ARCH_HANDLER_H

#include "arch_trap/parser.h"
#include "types.h"

void trapframe_user_init(trapframe *trapframe, u64 start_location);
void attach_trap_handler();

#endif