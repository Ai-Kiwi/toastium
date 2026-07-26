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

#define VMA_DONT_COMPRESS  (1 << 8)

void vma_map_kernel(process *proc, u64 virt_addr, u64 size, u64 phys_addr, u64 access_flags);
void vma_map_user(process *proc, u64 virt_addr, u64 size, u64 phys_addr, u64 access_flags);
void vma_unmap(process *proc, u64 virt_addr, u64 size);

void vma_swap(process *process);
void vma_create(process *proc);
void vma_init();

void vma_enable_read_user();
void vma_disable_read_user();

#endif
