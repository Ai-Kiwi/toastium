#include "arch_trap/irq.h"
#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "kernel/process/process.h"

void kernel_trap_process_kernel() {
    //need to some how get process

    arch_trapframe *process_trapframe = (arch_trapframe *)TRAPFRAME_ADDRESS;
    kernel_process *process = (kernel_process *)process_trapframe->process_ptr;



    //before actually switching trapframes to kernel one figure out what todo with old data.




    arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->kernelspace_trap_frame, VMA_READ | VMA_WRITE);
    //change process state to say that its runnning as 
    arch_irq_enable();

    //instead will store info in trap frames
    //will also need changing when trapframes change.

    //satp : argument
    //hart id : tp


    arch_irq_disable();
    arch_vma_assign_kernel(process, TRAPFRAME_ADDRESS, (u64)process->userspace_trap_frame, VMA_READ | VMA_WRITE);
    //store process as running in user state
}