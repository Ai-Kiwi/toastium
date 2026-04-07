.section .text
.global trap_entry

.include "./src/arch/risc-v/state.s"

trap_entry:
    SAVE_REGISTERS_TO_STACK
    


    LOAD_REGISTERS_FROM_STACK