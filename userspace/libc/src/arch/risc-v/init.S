.section .text
.global __init_c_env
.type __init_c_env,%function

__init_c_env:
    //clear out the bss
    la t0, __bss_start
    la t1, __BSS_END__
    j clear_bss

clear_bss:
    bgeu t0, t1, start_os
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss