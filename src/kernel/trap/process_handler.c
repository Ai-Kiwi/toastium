#include "arch_trap/irq.h"
#include "arch_trap/parser.h"
#include "board.h"
#include "kernel/process/process.h"

void kernel_trap_process_kernel() {
    //need to some how get process

    //process_info *pointer_process = (process_info *)process_trapframe->process_pointer;
    arch_trapframe *process_trapframe = (arch_trapframe *)TRAPFRAME_ADDRESS;


    u64 virtual_memory_root_table =
    u64 virtual_memory_process_number_cache = 

    //before actually switching trapframes to kernel one figure out what todo with old data.




    arch_virtual_memory_assign_kernel_page(process, TRAPFRAME_ADDRESS, (u64)process->kernelspace_trap_frame, VMA_READ | VMA_WRITE);
    //change process state to say that its runnning as 
    arch_irq_enable();

    //instead will store info in trap frames
    //will also need changing when trapframes change.

    //satp : argument
    //hart id : tp


    arch_irq_disable();
    arch_virtual_memory_assign_kernel_page(process, TRAPFRAME_ADDRESS, (u64)process->userspace_trap_frame, VMA_READ | VMA_WRITE);
    //store process as running in user state
}