#include "layout.h"
#include "state.S"
.section .text.trap
.global trap_entry
.global after_trap_hold
.align 4

trap_entry:
    STORE_TRAP_FRAME #TODO: make this include floating point, also 90% sure stack ptr won't work propperly

    #swap to stack for kernel
    li t0, TRAPFRAME_ADDRESS
    la sp, _hart_stacks_start
    li sp, HART_KERN_STACK_TOP

    # setup restore once done
    li a0, TRAPFRAME_ADDRESS
    call handle_sync_trap #outputs to a0 response

    bne a0, zero, run_process_kernel_trap


    j resume_process

run_process_kernel_trap:
    #swap to process kernel stack
    mv sp, a0 #sets to response stack location
    call handle_async_trap
    j resume_process


resume_process:
    #loads back in old stack as well
    LOAD_TRAP_FRAME
    sret
