#include "arch_trap/irq.h"
#include "arch_trap/parser.h"
#include "arch_vma/virtual_memory.h"
#include "board.h"
#include "drivers/uart/uart.h"
#include "include/types.h"
#include "kernel/process/process.h"
#include "include/types.h"

extern u64 arch_read_user(u64 src, u64 size, u64 dest);

bool8 kernel_read_user(u64 src, u64 size, u64 dest) {
    if (src >= KERNEL_VMA_START || src + size >= KERNEL_VMA_START || src + size <= src) {
        return FALSE;
    }

    kernel_process *process = (kernel_process *)((arch_trapframe *)TRAPFRAME_ADDRESS)->process_ptr;
    arch_irq_disable();
    arch_vma_enable_read_user();
    u64 response = arch_read_user(src, size, dest);

    arch_vma_disable_read_user();
    arch_irq_enable();
    return !response;
}

