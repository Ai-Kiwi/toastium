#include "board.h"
#include "drivers/uart/uart.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/syscall/handler.h"
#include "kernel/trap/handler.h"
#include "kernel/trap/process_handler.h"
#include "types.h"
#include "kernel/trap/page_fault/handler.h"
#include "page_fault.h"

void interruptable_trap_page_fault(trap_data *trap, interruptable_trap_response *response) {
    process *proc = (process *)trap->process_ptr;
    bool8 page_loaded = pgfault_load(trap);
    if (proc->trap_state == PROC_TRAP_PROCESS_READ_USERSPACE) {
        response->queued_response = !page_loaded;
        response->send_response = !page_loaded;
        response->skip_instruction = !page_loaded;
    }else{
        //was userspace that called it
        if (page_loaded == FALSE) {
            uart_println_str("process set to be killed. Invalid page for page fault");
            //long term would return a response instead of killing
            response->kill_process = TRUE;
        }
    }
}



