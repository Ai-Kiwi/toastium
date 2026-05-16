#include "process.h"
#include "kernel/trap/handler.h"
#include "def.h"

void init_processes() {
    for (s32 i=0; i<max_process_count; i++) {
        kernel_process_info process;
        process.alive = TRUE;
        //kernel_processes[i] = process;
    }

    //TODO: create and setup a idle process at 0. Will do nothing except forever loop waiting for interrupt.
}

void start_process() {

}


void kill_process(pid process_id) {
    //arch_processes_trap_info[kernel_process_id] = ;

}




