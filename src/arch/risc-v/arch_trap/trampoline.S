#include "state.S"


#save to state to processor kernel core stack

#run trap parser

#decided is it wants to run on kernel core stack or process stack
#returns which stack to use and which c to run

#Writes to whatever tramp reg data was put in, from vma.

#       (c) Kernel Stack
#Runs parse function gets generic reg data (arch generic function)
#Handles what todo
#assign vma reg tramp for next process (kernel or process)
#run process exit handle
#return to asm after

#       (c) Process tack
#Runs parse function gets generic reg data (arch generic function)
#Handles what todo
#assign vma reg tramp for next process (kernel or process)
#run process exit handle
#return to asm after

#       (c) process exit handle
#setup vma

#       (asm) Asm after
#restore reg from what is stored
#run resume process




trap_entry:
    li t0, (0x10000000 + KERNEL_VMA_START)
    li t1, 'T'
    sb t1, 0(t0)

    STORE_TRAP_FRAME

    call kernel_handle_trap #c handler. Dumps response to a0

    bgtz a0, process_trap_handler
    j trap_exit

process_trap_handler:
    mv sp, a0
    call kernel_trap_process_kernel
    j trap_exit

trap_exit:
    LOAD_TRAP_FRAME
    sret








