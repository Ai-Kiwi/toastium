.section .text.idle_process
.global idle_process_entry
.align 4

idle_process_entry:
    
    li a0, 1
    li a1, 0

    li a2, 'h'
    ecall
    li a2, 'e'
    ecall
    li a2, 'l'
    ecall
    li a2, 'l'
    ecall
    li a2, 'o'
    ecall
    li a2, ' '
    ecall
    li a2, 'w'
    ecall
    li a2, 'o'
    ecall
    li a2, 'r'
    ecall
    li a2, 'l'
    ecall
    li a2, 'd'
    ecall
    li a2, '\n'
    ecall
    #wfi
    j idle_process_entry