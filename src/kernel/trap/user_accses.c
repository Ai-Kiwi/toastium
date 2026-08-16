#include "user_accses.h"
#include "arch_trap/irq.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "include/types.h"
#include "kernel/process/process.h"

extern u64 arch_read_user(u64 src, u64 size, u64 dest);

bool8 kernel_read_user(u64 src, u64 size, u64 dest, process *proc) {
    if (src >= KERNEL_VMA_START || src + size >= KERNEL_VMA_START ||
        src + size <= src) {
        return FALSE;
    }

    // process *proc = (process *)((trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    irq_disable();
    vma_enable_read_user();
    proc->reading_userspace = TRUE;
    u64 response = arch_read_user(src, size, dest);
    proc->reading_userspace = FALSE;
    vma_disable_read_user();
    irq_enable();
    return !response;
}
