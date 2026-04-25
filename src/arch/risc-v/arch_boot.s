.section .text.boot
.global _start

_start:
    .option norvc
    j real_start #code 1
    .option rvc
    .long 0 # code 2
    .quad 0x200000 # image load offset (little endian)
    .quad _unused_ram_start - _kernel_ram_start #image size, little endian
    .quad 0 #kernel flags, little endian
    .long 2 #version of header
    .long 0 #reserved
    .quad 0 #reserved
    .quad 0x5643534952 #magic meaning "RISCV", little endian
    .long 0x05435352 #magic 2 meaning "RSC\x05", little endian
    .long 0 #PE COFF offset

real_start:
    fence.i

    #setup stack top
    la sp, _stack_top

    #bind for interrupts 
    la t0, trap_entry
    csrw stvec, t0
    
    #clear out the bss
    la t0, _bss_bottom
    la t1, _bss_top
    j clear_bss
clear_bss:
    bgeu t0, t1, clear_bss_done 
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss
clear_bss_done:
    call arch_main
    wfi
