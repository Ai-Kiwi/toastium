.section .text.trap
.global trap_entry
.global after_trap_hold
.align 4
.include "./src/arch/risc-v/state.s"

trap_entry:
    SAVE_REGISTERS_TO_STACK

    # setup restore once done
    mv a0, sp
    call arch_trap_handler

    LOAD_REGISTERS_FROM_STACK
    sret

after_trap_hold:
    wfi
    j after_trap_hold
