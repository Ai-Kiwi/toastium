.section .text
.global _start
.type _start,%function
#start of libc, will setup env then call process itself
_start:
    mv sp, a0
    #setup bss
    #setup regs
    #call main libc start. From there will do propper starts
    call main
