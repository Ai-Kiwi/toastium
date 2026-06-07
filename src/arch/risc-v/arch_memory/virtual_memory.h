#ifndef ARCH_VIRTUAL_MEMORY_H
#define ARCH_VIRTUAL_MEMORY_H

#include "types.h"
#include "kernel/process/process.h"

void arch_virtual_memory_assign_kernel_page(process_info process, u64 virtual_address, u64 physical_address, u64 argument_flags);
void arch_virtual_memory_assign_user_page(process_info process, u64 virtual_address, u64 physical_address, u64 argument_flags);

void arch_virtual_memory_change_table(process_info process);

#endif
