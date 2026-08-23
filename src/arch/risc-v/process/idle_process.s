.section .text.idle_process
.global idle_process_entry
.align 4

idle_process_entry:
    li a0, 3
    ecall

    #wfi
    j idle_process_entry
