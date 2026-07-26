#ifndef KERNEL_TRAP_PAGE_FAULT_H
#define KERNEL_TRAP_PAGE_FAULT_H

#include "kernel/trap/handler.h"
#include "types.h"

bool8 pgfault_load(trap_data *trap_data);

#endif