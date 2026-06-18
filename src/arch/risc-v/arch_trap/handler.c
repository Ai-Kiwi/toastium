
#include "arch_trap/parser.h"

#include "types.h"

void arch_trapframe_init_user(arch_trapframe *trapframe, u64 start_location) {
    u64 is_supervisor = 0;

    trapframe->sstatus = (is_supervisor << 8);
    trapframe->sepc = start_location;
}

