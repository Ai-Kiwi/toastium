#ifndef ARCH_UNSAFE_TRAP_HANDLER_H
#define ARCH_UNSAFE_TRAP_HANDLER_H

#include "types.h"

typedef struct {
    u64 scause;
    u64 sepc;
    u64 stval;
    u64 sstatus;
} raw_trap_info;

void handle_unsafe_trap(raw_trap_info *trap);

#endif
