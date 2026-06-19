.section .text.idle_process
.global idle_process_entry
.align 4

idle_process_entry:
    li a0, 'h'
    ecall
    li a0, 'e'
    ecall
    li a0, 'l'
    ecall
    li a0, 'l'
    ecall
    li a0, 'o'
    ecall
    li a0, ''
    ecall
    li a0, 'w'
    ecall
    li a0, 'o'
    ecall
    li a0, 'r'
    ecall
    li a0, 'l'
    ecall
    li a0, 'd'
    ecall
    #wfi
    j idle_process_entry