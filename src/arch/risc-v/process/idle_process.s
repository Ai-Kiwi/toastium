.section .text.idle_process
.global idle_process_entry

idle_process_entry:
    #ecall
    wfi
    j idle_process_entry