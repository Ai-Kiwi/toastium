
#include "handler.h"
#include "arch_trap/parser.h"
#include "types.h"

extern u8 trap_entry;

// sets up basic info like start location for a process.
// designed for basic stating process. This is all handled by elf parser for
// anything more
void trapframe_user_init(trapframe *trapframe, u64 start_location) {
    u64 is_supervisor = 0;

    trapframe->sstatus = (is_supervisor << 8);
    trapframe->sepc = start_location;
}

void attach_trap_handler() {
    asm volatile("csrw stvec, %0" ::"r"(&trap_entry) : "memory");
}
