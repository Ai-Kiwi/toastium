.section .text.idle_process
.global idle_process_entry

idle_process_entry:
    wfi
    j idle_process_entry