#ifndef ARCH_VMA_H
#define ARCH_VMA_H

#include "types.h"
#include "kernel/process/process.h"

#define VMA_READ  (1 << 1)
#define VMA_WRITE (1 << 2)
#define VMA_EXEC  (1 << 3)

#define VMA_VALID (1 << 0)
#define VMA_USER  (1 << 4)
#define VMA_GLOBAL  (1 << 5)

void arch_vma_assign_kernel(u64 page_table, u64 addr_space_id, u64 virt_addr, u64 phys_addr, u64 arg_flags);
void arch_vma_assign_user(u64 page_table, u64 addr_space_id, u64 virt_addr, u64 phys_addr, u64 arg_flags);

#endif
