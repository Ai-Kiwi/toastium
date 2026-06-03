#include "arch_trap/handler.h"
#include "arch_trap/irq.h"

void kernel_trap_process_kernel() {
    arch_trap_frame_set_process();
    //change process state to say that its runnning as 
    arch_irq_enable();



    arch_irq_disable();
    arch_trap_frame_set_kernel();
    //store process as running in user state
}