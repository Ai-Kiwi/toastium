#include "user_accses.h"
#include "arch_trap/irq.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "include/types.h"
#include "kernel/process/process.h"

extern u64 arch_copy_user(u64 src, u64 size, u64 dest);

bool8 kernel_read_user(u64 src, u64 size, void *dest, process *proc) {
    if (src >= KERNEL_VMA_START || src + size >= KERNEL_VMA_START ||
        src + size <= src) {
        return FALSE;
    }

    // process *proc = (process *)((trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    irq_disable();
    vma_enable_read_user();
    proc->reading_userspace = TRUE;
    u64 response = arch_copy_user(src, size, (u64)dest);
    proc->reading_userspace = FALSE;
    vma_disable_read_user();
    irq_enable();
    return !response;
}

bool8 kernel_write_user(u64 dest, u64 size, void *src, process *proc) {
    if (dest >= KERNEL_VMA_START || dest + size >= KERNEL_VMA_START ||
        dest + dest <= dest) {
        return FALSE;
    }

    // process *proc = (process *)((trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    irq_disable();
    vma_enable_read_user();
    proc->reading_userspace = TRUE;
    u64 response = arch_copy_user((u64)src, size, dest);
    proc->reading_userspace = FALSE;
    vma_disable_read_user();
    irq_enable();
    return !response;
}
