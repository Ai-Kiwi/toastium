#include "board.h"
#include "state.S"
.section .text.trap
.global trap_entry
.global after_trap_hold
.align 4

trap_entry:
    STORE_TRAP_FRAME #TODO: make this include floating point, also 90% sure stack ptr won't work propperly

    # setup restore once done
    mv a0, sp
    call kernel_handle_trap #outputs to a0 response

    bgtz a0, run_process_kernel_trap


    j resume_process

run_process_kernel_trap:
    call 


resume_process:
    LOAD_TRAP_FRAME
    sret
