.section .text.trap
.global trap_entry
.align 4
.include "./src/arch/risc-v/state.s"

trap_entry:
    SAVE_REGISTERS_TO_STACK

    #setup restore once done
    call arch_trap_handler

    LOAD_REGISTERS_FROM_STACK
    sret
