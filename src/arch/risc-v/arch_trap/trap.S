#include "board.h"
#include "state.S"
.section .text.trap
.global trap_entry
.global after_trap_hold
.align 4

trap_entry:
    STORE_TRAP_FRAME #TODO: make this include floating point, also 90% sure stack ptr won't work propperly

    #swap to stack for kernel

    # setup restore once done
    li a0, TRAPFRAME_ADDRESS
    mv sp, a0
    call kernel_handle_trap #outputs to a0 response

    bgtz a0, run_process_kernel_trap


    j resume_process

run_process_kernel_trap:
    #swap to process kernel stack
    mv sp, a0 #sets to response stack location
    call kernel_trap_process_kernel


resume_process:
    #loads back in old stack as well
    LOAD_TRAP_FRAME
    sret
