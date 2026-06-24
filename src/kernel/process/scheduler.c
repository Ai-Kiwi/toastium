#include "arch_vma/virtual_memory.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/trap/handler.h"
#include "def.h"
#include "kernel/timer/timer.h"
#include "types.h"
#include "kernel/memory/pager.h"
#include "kernel/safety/panic.h"

//almost all the code in here atm is temp and will be completely rewritten.
//this is just a round robin schedular so i am able to get it working for testing.

//also none of this is fast or safe for SMP. THis doesn't matter as it is just planning code
//it will be replaced before smp gets added.

volatile u64 *process_queue; //page storing ring buffer of processes
u64 process_queue_size;
u64 process_queue_count;
u64 process_queue_start;

pid kernel_running_process;

kernel_process **hart_idle_process;

//this also marks current as running so must be careful handled
//also after remove has to be readded
kernel_process *kernel_scheduler_dequeue_next_process(u64 hart_id) {
    if (process_queue_count == 0) {
        return hart_idle_process[hart_id];
    }
    kernel_process *output_process = (kernel_process *)process_queue[process_queue_start];
    process_queue_start = (process_queue_start + 1) % process_queue_size;

    process_queue_count--;
    return output_process;
}

void kernel_scheduler_queue_process(kernel_process *process) {
    if (process_queue_size == process_queue_count) {
        PANIC("KERNEL_SCHEDULER_QUEUE_FULL", 0, 0, 0);
    }
    process_queue[(process_queue_start + process_queue_count) % process_queue_size] = (u64)process;
    process_queue_count++;
}

void kernel_scheduler_dequeue_process(kernel_process *process) {
    u64 offset = 0;
    for (u64 i=1; i<process_queue_count+1; i++) {
        if (process_queue[(process_queue_start + (process_queue_count - i)) % process_queue_size] == (u64)process) {
            offset--;
        }
        if (offset==0) {continue;}

        process_queue[(process_queue_start + (process_queue_count - i)) % process_queue_size] = process_queue[(process_queue_start + (process_queue_count - i) + offset) % process_queue_size];
    }
}

void kernel_schedular_init(u64 hart_count) {
    //kernel_running_process = null_program_pid;
    process_queue = (u64 *)kernel_pager_acquire();
    process_queue_size = KERNEL_PAGE_SIZE / 8;
    process_queue_count = 0;
    process_queue_start = 0;
    //make sure to set process trapframe hart_id to correct value, both kernel and userspace

    if (hart_count > (KERNEL_PAGE_SIZE / 8)) {
        PANIC("TO_MANY_IDLE_PROCESSES_FOR_PAGE", 0, 0, 0);
    }
    hart_idle_process = (kernel_process **)kernel_pager_acquire();

    for (u64 i=0; i<hart_count; i++) {
        hart_idle_process[i] = (kernel_process *)kernel_process_from_id(i);
    }
}