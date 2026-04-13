#include "process.h"
#include "kernel/trap/handler.h"
#include "def.h"

kernel_process_info kernel_processes[max_process_count];

void init_processes() {
    for (int i=0; i<max_process_count; i++) {
        kernel_process_info process;
        process.alive = TRUE;
        kernel_processes[i] = process;
    }
}

void start_process() {

}


void kill_process(kpid kernel_process_id) {
    //arch_processes_trap_info[kernel_process_id] = ;

}




