.section .text.init_process
.global init_process_entry
.align 4

init_process_entry:
    li a0, 1
    li a1, 0


    li a2, 'i'
    ecall
    li a2, 'n'
    ecall
    li a2, 'i'
    ecall
    li a2, 't'
    ecall
    li a2, '\n'
    ecall
    #wfi
    j init_process_entry