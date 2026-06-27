#include "process.h"
#include "arch_trap/parser.h"
#include "drivers/uart/uart.h"
#include "kernel/memory/allocator.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/memory/radix.h"
#include "kernel/safety/panic.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "kernel/memory/pager.h"
#include "arch_trap/handler.h"
#include "types.h"
#include "kernel/process/scheduler.h"
#include <stdalign.h>

//upto 65,536 processes
#define pid_level_size 4
#define pid_levels 4
#define max_processes 65536

extern u8 _kernel_idle_process, _kernel_init_process;

//not 64 bytes aligned for multicore
alignas(64) u64 __attribute__((aligned(64))) process_upto = 0;
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
    new_process.trap_state = KPROC_TRAP_PROCESS;
    new_process.block_waiting = 0;
    new_process.process_id = process_upto;
    new_process.vma_addr_space_id = U64_MAX;
    new_process.vma_table = (u64 *)arch_vma_create();
    new_process.userspace_trap_frame = (arch_trapframe *)kernel_pager_acquire();
    new_process.kernelspace_trap_frame = (arch_trapframe *)kernel_pager_acquire();
    new_process.page_fault_trap_frame = (arch_trapframe *)kernel_pager_acquire();
    new_process.process_type = KPROC_TYPE_NORMAL;
    new_process.running = FALSE;
    new_process.handles_page = 0;
    arch_vma_assign_kernel((kernel_process *)&new_process, TRAPFRAME_ADDRESS, (u64)new_process.userspace_trap_frame, VMA_READ | VMA_WRITE);

    u64 kernel_stack = kernel_pager_acquire();
    new_process.kernel_stack = kernel_stack;

    kernel_process *process = (kernel_process *)kernel_allocator_acquire(sizeof(kernel_process));
    new_process.userspace_trap_frame->process_ptr = (u64)process;
    new_process.kernelspace_trap_frame->process_ptr = (u64)process;
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
        idle_process->process_type = KPROC_TYPE_IDLE;
        arch_trapframe_init_user(idle_process->userspace_trap_frame, 0x1000);
        arch_vma_assign_user(idle_process, 0x1000, (u64)&_kernel_idle_process, VMA_EXEC);
    }
}

void kernel_process_iter(void (*function)(u64, u64), u64 parameters) {
    kernel_radix_iter_children((u64)process_radix_root, pid_levels, pid_level_size, parameters, function);
}

void kernel_process_kill_process(pid process_id) {
    kernel_process *process = kernel_process_from_id(process_id);
    if (process->process_type == KPROC_TYPE_IDLE) {
        PANIC("ATTEMPT_TO_KILL_IDLE_PROCESS", 0, 0, 0);
    }
    if (process->running == TRUE) {
        process->process_type = KPROC_TYPE_DEAD;
    }else{
        kernel_scheduler_dequeue_process(process);
        kernel_process_cleanup_process(process);
    }
}

void kernel_process_cleanup_process(kernel_process *process) {
    kernel_radix_remove_child((u64)process_radix_root, process->process_id, pid_levels, pid_level_size);
    kernel_pager_release((u64)process->userspace_trap_frame);
    kernel_pager_release((u64)process->kernelspace_trap_frame);
    kernel_pager_release((u64)process->vma_table);
    kernel_pager_release(process->kernel_stack); //on large page this will need to be changed to release all of them not just first
    kernel_allocator_release((u64)process);
}


void kernel_process_create_init_process() {
    //create the init process
    kernel_process *init_process = new_blank_process();
    arch_trapframe_init_user(init_process->userspace_trap_frame, 0x1000);
    arch_vma_assign_user(init_process, 0x1000, (u64)&_kernel_init_process, VMA_EXEC | VMA_READ);

    kernel_scheduler_queue_process(init_process);
}