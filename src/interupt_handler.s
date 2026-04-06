.section .text
.global trap_entry

.include "./src/state_handler.s"

trap_entry:
    SAVE_REGISTERS_TO_STACK
    


    LOAD_REGISTERS_FROM_STACK