#include "kernel/safety/panic.h"

void arch_trap_frame_set_process() {
    //swaps interrupt vma to process stack
    PANIC("NOT_CODED_arch_trap_frame_set_process",0,0,0);
}

void arch_trap_frame_set_kernel() {
    //swaps interrupt vma to process kernel stack
    PANIC("NOT_CODED_arch_trap_frame_set_kernel",0,0,0);
}