.section .text.boot
.global _start

_start:
    #setup stack top
    la sp, _stack_top

    #bind for interrupts 
    #la t0, trap_entry
    #csrw mtvec, t0
    
    #clear out the bss
    la t0, _bss_bottom
    la t1, _bss_top
    j clear_bss
clear_bss:
    beq t0, t1, clear_bss_done 
    sd zero, 0(t0)
    addi t0, t1, 8
    j clear_bss
clear_bss_done:
    call kernel_main
    
1: j 1b
