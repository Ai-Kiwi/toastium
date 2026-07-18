#include "process.h"
#include "arch_trap/parser.h"
#include "drivers/uart/uart.h"
#include "kernel/memory/allocator.h"
#include "kernel/memory/hashmap.h"
#include "kernel/trap/handler.h"
#include "def.h"
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
alignas(64) hashmap __attribute__((aligned(64))) process_hashmap;

process *process_from_id(pid process_id) {
    return (process *)hashmap_get(&process_hashmap, (u64)process_id);
}

process *new_blank_process() {
    process *current_process; //Non zero
    while (TRUE){
        current_process = (process *)hashmap_get(&process_hashmap,process_upto);
        if (!current_process) {
            break;
        }
        process_upto++;
    }
    process new_process;
    new_process.trap_state = PROC_TRAP_PROCESS;
    new_process.block_waiting = 0;
    new_process.process_id = process_upto;
    new_process.vma_addr_space_id = U64_MAX;
    new_process.vma_table = (u64 *)vma_create();
    new_process.userspace_trapframe = (trapframe *)pg_alloc();
    new_process.kernelspace_trapframe = (trapframe *)pg_alloc();
    new_process.page_fault_trapframe = (trapframe *)pg_alloc();
    new_process.process_type = PROC_TYPE_NORMAL;
    new_process.running = FALSE;
    new_process.file_desc_cnt = 0;
    new_process.file_desc_pg = (u64 *)pg_alloc();
    new_process.phys_kernel_stack_addr[0] = pg_alloc();
    new_process.phys_kernel_stack_addr[1] = pg_alloc();
    new_process.phys_kernel_stack_addr[2] = pg_alloc();
    new_process.phys_kernel_stack_addr[3] = pg_alloc();
    vma_assign_kernel((process *)&new_process, TRAPFRAME_ADDRESS, (u64)new_process.userspace_trapframe, VMA_READ | VMA_WRITE);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START-KERNEL_PAGE_SIZE, 0x0, 0);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START, (u64)new_process.phys_kernel_stack_addr[0], VMA_READ | VMA_WRITE);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START+(KERNEL_PAGE_SIZE), (u64)new_process.phys_kernel_stack_addr[1], VMA_READ | VMA_WRITE);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START+(KERNEL_PAGE_SIZE*2), (u64)new_process.phys_kernel_stack_addr[2], VMA_READ | VMA_WRITE);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START+(KERNEL_PAGE_SIZE*3), (u64)new_process.phys_kernel_stack_addr[3], VMA_READ | VMA_WRITE);
    vma_assign_kernel((process *)&new_process, PROCESS_KERNEL_STACK_START+(KERNEL_PAGE_SIZE*4), 0x0, 0);

    process *temp_process = (process *)mem_alloc(sizeof(process));
    new_process.userspace_trapframe->process_ptr = (u64)temp_process;
    new_process.kernelspace_trapframe->process_ptr = (u64)temp_process;
    *temp_process = new_process;



    u64 old_child = (u64)hashmap_insert(&process_hashmap, process_upto, (u64)temp_process);
    if (old_child) {
        PANIC("CREATE_BLANK_PROCESS_CONFLICTING_CHILD_PRESENT",old_child,0,0);
    }

    process_upto++;//increase for next process
    return (process *)temp_process;
}


void processes_init(u64 hart_count) {
    hashmap_create(HASHMAP_TYPE_NUMBER, &process_hashmap);
    process_hashmap.start = (u64 *)pg_alloc();
    process_hashmap.len = KERNEL_PAGE_SIZE / 8;

    process_upto = 0;

    for (u64 hart_id = 0; hart_id<hart_count; hart_id++) {
        process *idle_process = new_blank_process();
        if (hart_id != idle_process->process_id) {
            PANIC("IDLE_PROCESS_INVALID_ID",idle_process->process_id, hart_id,0);
        }
        idle_process->runing_hart_id = hart_id;
        idle_process->process_type = PROC_TYPE_IDLE;
        trapframe_user_init(idle_process->userspace_trapframe, 0x1000);
        vma_assign_user(idle_process, 0x1000, (u64)&_kernel_idle_process, VMA_EXEC);
    }
}

void processes_iter(void (*function)(u64, u64), u64 parameters) {
    //move to hashmap, currently no way to loop will add later after working
    PANIC("LOOPING_OVER_PROCESSES_NOT_IMPLEMTED", 0, 0, 0)
    //kernel_radix_iter_children((u64)process_radix_root, pid_levels, pid_level_size, parameters, function);
}

void kill_process(pid process_id) {
    process *proc = process_from_id(process_id);
    if (proc->process_type == PROC_TYPE_IDLE) {
        PANIC("ATTEMPT_TO_KILL_IDLE_PROCESS", 0, 0, 0);
    }
    if (proc->running == TRUE) {
        proc->process_type = PROC_TYPE_DEAD;
    }else{
        scheduler_remove(proc);
        process_cleanup(proc);
    }
}

void process_cleanup(process *proc) {
    hashmap_remove(&process_hashmap, proc->process_id);
    pg_free((u64)proc->userspace_trapframe);
    pg_free((u64)proc->kernelspace_trapframe);
    pg_free((u64)proc->vma_table);
    pg_free((u64)proc->phys_kernel_stack_addr[0]);
    pg_free((u64)proc->phys_kernel_stack_addr[1]);
    pg_free((u64)proc->phys_kernel_stack_addr[2]);
    pg_free((u64)proc->phys_kernel_stack_addr[3]);
    mem_free((u64)proc);
}


void create_init_process() {
    //create the init process
    process *init_process = new_blank_process();
    trapframe_user_init(init_process->userspace_trapframe, 0x1000);
    vma_assign_user(init_process, 0x1000, (u64)&_kernel_init_process, VMA_EXEC | VMA_READ);

    scheduler_queue_process(init_process);
}