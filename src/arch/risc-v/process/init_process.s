.section .text.init_process
.global init_process_entry
.align 4

init_process_entry:
    li a3, 0x123
    li a0, 'i'
    ecall
    li a0, 'n'
    ecall
    li a0, 'i'
    ecall
    li a0, 't'
    ecall
    li a0, '\n'
    ecall
    #wfi
    j init_process_entry