#include "board.h"
#include "state.S"
.section .text.unsafe_trap
.global unsafe_trap_entry
.align 4

#handles unsafe trap
#these happen during a core stat, idea is it is before processes are started so normal trap handler would fail

unsafe_trap_entry:
    #swap to stack for kernel
    la sp, _stack_top
    csrr a0, scause
    csrr a1, sepc
    csrr a2, stval
    csrr a3, sstatus
    sd a0, 0(sp) #tf->scause
    sd a1, 8(sp) #tf->sepc
    sd a2, 16(sp) #tf->stval
    sd a3, 24(sp) #tf->sstatus

    addi a0, sp, 0
    addi sp, sp, -32

    call handle_unsafe_trap #handle an unsafe trap