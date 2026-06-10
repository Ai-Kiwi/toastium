#ifndef ARCH_VIRTUAL_MEMORY_H
#define ARCH_VIRTUAL_MEMORY_H

#include "types.h"
#include "kernel/process/process.h"

#define VMA_READ  (1 << 1)
#define VMA_WRITE (1 << 2)
#define VMA_EXEC  (1 << 3)

#define VMA_VALID (1 << 0)
#define VMA_USER  (1 << 4)
#define VMA_GLOBAL  (1 << 5)

void arch_virtual_memory_assign_kernel_page(u64 root_table, u64 process_cache_number, u64 virtual_address, u64 physical_address, u64 argument_flags);
void arch_virtual_memory_assign_user_page(u64 root_table, u64 process_cache_number, u64 virtual_address, u64 physical_address, u64 argument_flags);

#endif
