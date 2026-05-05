.section .text.idle_process

idle_process:
    wfi
    j idle_process