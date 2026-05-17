#include "process.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/memory/radix.h"
#include "kernel/safety/panic.h"

//upto 65,536 processes
#define pid_level_depth 4
#define pid_levels 4
#define max_processes 65536

u64 process_upto = 0;
u64 *process_radix_root = 0;

process_info *kernel_process_from_id(pid process_id) {

}

process_info *new_blank_process() {
    process_info *current_process; //Non zero
    while (TRUE){
        current_process = (process_info *)kernel_radix_get_child((u64)process_radix_root, process_upto, pid_levels, pid_level_depth);
        if (!current_process) {
            break;
        }
        process_upto++;
    }
    process_info new_process;
    new_process.process_id = process_upto;

    process_info *process = (process_info *)kernel_allocator_acquire(sizeof(process_info));
    *process = new_process;

    u64 old_child = (u64)kernel_radix_create_child((u64)process_radix_root,process->process_id,(u64)process,pid_levels,pid_level_depth);
    if (old_child) {
        PANIC("CREATE_BLANK_PROCESS_CONFLICTING_CHILD_PRESENT",old_child,0,0);
    }

    process_upto++;//increase for next process
    return (process_info *)process;
}


void init_processes() {
    process_radix_root = (u64 *)kernel_radix_create_tree(pid_level_depth);


    process_upto = 0;
    //process 0 is idle process
    process_info *idle_process = new_blank_process();
    if (idle_process->process_id != 0) {
        PANIC("IDLE_PROCESS_NO_ZERO_ID",idle_process->process_id,0,0);
    }

    //TODO: setup vma table fot this idea process

    //TODO: create and setup a idle process at 0. Will do nothing except forever loop waiting for interrupt.
}

void start_process() {

}


void kill_process(pid process_id) {
    //arch_processes_trap_info[kernel_process_id] = ;

}




