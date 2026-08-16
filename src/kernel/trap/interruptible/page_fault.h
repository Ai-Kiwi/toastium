#ifndef KERNEL_TRAP_INTERRUPTABLE_PAGE_FAULT_H
#define KERNEL_TRAP_INTERRUPTABLE_PAGE_FAULT_H

#include "kernel/trap/handler.h"
#include "kernel/trap/process_handler.h"

void interruptable_trap_page_fault(trap_data *trap,
                                   interruptable_trap_response *response);

#endif