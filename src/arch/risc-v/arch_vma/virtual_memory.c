//following code assumes sv39. This os realistically isn't going past that.
//everything in here is self contained assumptions, if in future more is needed
//this file can be swapped out or replaced with generic middle-man.

#include "drivers/uart/uart.h"
#include "types.h"
#include "def.h"
#include "kernel/memory/pager.h"
#include "kernel/safety/panic.h"
#include "kernel/process/process.h"
#include "virtual_memory.h"
#include "board.h"

//not 64 bytes aligned for multicore
u64 max_asid;
u64 current_highest_asid;

u64 arch_vma_create() {
    u64 new_page_addr = kernel_pager_acquire();

    //create kernel mapping
    ///allow read/write/execute, mark as global for optimization, is valid and also mark as already dirty and accessed for performance.
    const u64 access_mask = 0xEF;
    volatile u64 *entry_leaf = (u64 *)new_page_addr;
    for (u64 i=0; i<256; i++) {
        entry_leaf[256 + i] = (i << 28) | access_mask;
    }
    return new_page_addr;
}

//must be called when init vma has already been set
//Reason is it detects max vma
void arch_vma_init() {
    //find max ASID
    asm volatile ("csrr %0, satp" : "=r"(max_asid));
    max_asid = (max_asid >> 44) & (BIT(16)-1);
    //set to max so first loop resets all
    current_highest_asid = max_asid;
}

void delete_process_asid(u64 process_ptr, u64 parameter) {
    kernel_process *process = (kernel_process *)process_ptr;

    process->vma_addr_space_id = 0;
}

void arch_vma_reset_asid() {
    uart_println_str("reset all vma asid");
    //loop over all processes, set asid to -1 meaning not set.
    current_highest_asid = 0;
    asm volatile ("sfence.vma zero, zero" ::: "memory");

    kernel_process_iter(delete_process_asid, 0);
}

u64 arch_vma_fetch_asid() {
    if (current_highest_asid >= max_asid) {
        arch_vma_reset_asid();
        current_highest_asid = 0;
    }else{
        current_highest_asid++;
    }
    return current_highest_asid;
}

void arch_vma_assign(kernel_process *process, u64 virt_addr, u64 vma_phys_addr, u64 arg_flags) {
    u64 ppn0_offset; //least significant
    u64 ppn1_offset;
    u64 ppn2_offset; //most significant

    u64 phys_addr = vma_phys_addr - KERNEL_VMA_START;

    if (virt_addr % 4096 || phys_addr % 4096) {
        PANIC("VMA_ASSIGN_NOT_PAGE_ALIGN", (s64)process->vma_table, (s64)virt_addr, (s64)phys_addr);
    }

    u64 ranged_virt_addr = virt_addr;
    //move down if kernel space addr
    if (ranged_virt_addr > 0x4000000000) {
        if (ranged_virt_addr < 0xffffffc000000000) {
            PANIC("VMA_ASSIGN_BETWEEN_USERSPACE_AND_KERNELSPACE", (s64)process->vma_table, (s64)virt_addr, (s64)phys_addr)
        }
        ranged_virt_addr = ranged_virt_addr - (0xffffffc000000000 - 0x4000000000);
    }

    ppn0_offset = ranged_virt_addr / 4096;
    ppn1_offset = ppn0_offset / 512;
    ppn2_offset = ppn1_offset / 512;

    ppn0_offset = ppn0_offset % 512;
    ppn1_offset = ppn1_offset % 512;
    ppn2_offset = ppn2_offset % 512;

    u64 *ppn2_table = (u64 *)process->vma_table;
    if ((ppn2_table[ppn2_offset] & 0x1) == 0) {
        u64 new_page_addr = kernel_pager_acquire();
        new_page_addr = (new_page_addr - KERNEL_VMA_START) / 4096;
        ppn2_table[ppn2_offset] = 1;
        ppn2_table[ppn2_offset] |= new_page_addr << 10;
    }

    u64 ppn1_table_addr = (((ppn2_table[ppn2_offset] >> 10) & (BIT(27) - 1)) * 4096) + KERNEL_VMA_START;
    u64 *ppn1_table = (u64 *)ppn1_table_addr;
    if ((ppn1_table[ppn1_offset] & 0x1) == 0) {
        u64 new_page_addr = kernel_pager_acquire();
        new_page_addr = (new_page_addr - KERNEL_VMA_START) / 4096;
        ppn1_table[ppn1_offset] = 1;
        ppn1_table[ppn1_offset] |= new_page_addr << 10;
    }

    u64 ppn0_table_addr = (((ppn1_table[ppn1_offset] >> 10) & (BIT(27) - 1)) * 4096) + KERNEL_VMA_START;
    u64 *ppn0_table = (u64 *)ppn0_table_addr;

    //can use to handle if already in use
    //if (ppn0_table[ppn0_offset] & 0x1 == 0) {
    //
    //}

    ppn0_table[ppn0_offset] = ((phys_addr / 4096) << 10) | arg_flags | VMA_VALID;

    asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr), "r"(process->vma_addr_space_id) : "memory"); //push cache update, only to current asid
}

void arch_vma_assign_kernel(kernel_process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags) {
    arch_vma_assign(process, virt_addr, phys_addr, arg_flags);
}

void arch_vma_assign_user(kernel_process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags) {
    arch_vma_assign(process, virt_addr, phys_addr, arg_flags | VMA_USER);
}

void arch_vma_swap(kernel_process *process) {
    if (process->vma_addr_space_id == U64_MAX) {
        process->vma_addr_space_id = arch_vma_fetch_asid();
    }

    u64 satp_value = 0
    | (((u64)process->vma_addr_space_id) << 44) //ASID ID
    | (8UL << 60) // says its sv39
    | ((((u64)process->vma_table) - KERNEL_VMA_START) / 4096);

    asm volatile ("fence rw, rw");
    asm volatile ("fence.i");
    asm volatile ("csrw satp, %0" :: "r"(satp_value) : "memory");
    //might need fence here need to look more into it
}
