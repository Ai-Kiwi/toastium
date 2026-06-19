#include "process.h"
#include "arch_trap/parser.h"
#include "drivers/uart/uart.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/memory/radix.h"
#include "kernel/safety/panic.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "kernel/memory/pager.h"
#include "arch_trap/handler.h"

//upto 65,536 processes
#define pid_level_size 4
#define pid_levels 4
#define max_processes 65536

extern u8 _kernel_idle_process;

//not 64 bytes aligned for multicore
u64 process_upto = 0;
u64 *process_radix_root = 0;

kernel_process *kernel_process_from_id(pid process_id) {
    return (kernel_process *)kernel_radix_get_child((u64)process_radix_root, process_id, pid_levels, pid_level_size);
}

kernel_process *new_blank_process() {
    kernel_process *current_process; //Non zero
    while (TRUE){
        current_process = (kernel_process *)kernel_radix_get_child((u64)process_radix_root, process_upto, pid_levels, pid_level_size);
        if (!current_process) {
            break;
        }
        process_upto++;
    }
    kernel_process new_process;
    new_process.process_id = process_upto;
    new_process.vma_addr_space_id = U64_MAX;
    new_process.vma_table = (u64 *)arch_vma_create();
    new_process.userspace_trap_frame = (arch_trapframe *)kernel_pager_acquire();
    new_process.kernelspace_trap_frame = (arch_trapframe *)kernel_pager_acquire();
    arch_vma_assign_kernel((kernel_process *)&new_process, TRAPFRAME_ADDRESS, (u64)new_process.userspace_trap_frame, VMA_READ | VMA_WRITE);

    kernel_process *process = (kernel_process *)kernel_allocator_acquire(sizeof(kernel_process));
    *process = new_process;

    u64 old_child = (u64)kernel_radix_create_child((u64)process_radix_root,process->process_id,(u64)process,pid_levels,pid_level_size);
    if (old_child) {
        PANIC("CREATE_BLANK_PROCESS_CONFLICTING_CHILD_PRESENT",old_child,0,0);
    }

    process_upto++;//increase for next process
    return (kernel_process *)process;
}


void kernel_processes_init(u64 hart_count) {
    process_radix_root = (u64 *)kernel_radix_create_tree(pid_level_size);


    process_upto = 0;

    for (u64 hart_id = 0; hart_id<hart_count; hart_id++) {
        kernel_process *idle_process = new_blank_process();
        if (hart_id != idle_process->process_id) {
            PANIC("IDLE_PROCESS_INVALID_ID",idle_process->process_id, hart_id,0);
        }
        idle_process->runing_hart_id = hart_id;
        arch_trapframe_init_user(idle_process->userspace_trap_frame, 0x1000);
        arch_vma_assign_user(idle_process, 0x1000, (u64)&_kernel_idle_process, VMA_EXEC);
        u32 *data = (u32 *)&_kernel_idle_process;
    }

    //TODO: setup vma table fot this idea process

    //TODO: create and setup a idle process at 0. Will do nothing except forever loop waiting for interrupt.
}

void start_process() {

}

void kernel_process_iter(void (*function)(u64, u64), u64 parameters) {
    kernel_radix_iter_children((u64)process_radix_root, pid_levels, pid_level_size, parameters, function);
}

void kill_process(pid process_id) {
    //arch_processes_trap_info[kernel_process_id] = ;

}
