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

void vma_assign_kernel(process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags);
void vma_assign_user(process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags);

void vma_swap(process *process);
u64 vma_create();
void vma_init();

void vma_enable_read_user();
void vma_disable_read_user();

#endif
