//following code assumes sv39. This os realistically isn't going past that. 
//everything in here is self contained assumptions, if in future more is needed 
//this file can be swapped out or replaced with generic middle-man.

#include "types.h"
#include "def.h"
#include "kernel/memory/pager.h"
#include "kernel/safety/panic.h"
#include "kernel/process/process.h"
#include "virtual_memory.h"

u64 arch_virtual_memory_create_root_table() {
    u64 new_page_address = kernel_pager_acquire();
    return new_page_address;
}

void arch_virtual_memory_assign_page(u64 root_table, u64 process_asid, u64 virtual_address, u64 physical_address, u64 argument_flags) {
    u64 ppn0_offset; //least significant
    u64 ppn1_offset;
    u64 ppn2_offset; //most significant

    if (virtual_address % 4096) {
        PANIC("VMA_ASSIGN_NOT_PAGE_ALIGN", (s64)root_table, (s64)virtual_address, (s64)physical_address);
    }

    u64 ranged_virtual_address = virtual_address;
    //move down if kernel space address
    if (ranged_virtual_address > 0x4000000000) {
        if (ranged_virtual_address < 0xffffffc000000000) {
            PANIC("VMA_ASSIGN_BETWEEN_USERSPACE_AND_KERNELSPACE", (s64)root_table, (s64)virtual_address, (s64)physical_address)
        }
        ranged_virtual_address = ranged_virtual_address - (0xffffffc000000000 - 0x4000000000);
    }

    ppn0_offset = ranged_virtual_address / 4096;
    ppn1_offset = ppn0_offset / 512;
    ppn2_offset = ppn1_offset / 512;

    ppn0_offset = ppn0_offset % 512;
    ppn1_offset = ppn1_offset % 512;
    ppn2_offset = ppn2_offset % 512;

    u64 *ppn2_table = (u64 *)root_table;
    if (ppn2_table[ppn2_offset] & 0x1 == 0) {
        u64 new_page_address = kernel_pager_acquire();
        new_page_address = new_page_address / 4096;
        ppn2_table[ppn2_offset] = 1;
        ppn2_table[ppn2_offset] |= new_page_address << 10;
    }

    u64 ppn1_table_address = ((ppn2_table[ppn2_offset] >> 10) & (BIT(27) - 1)) * 4096;
    u64 *ppn1_table = (u64 *)ppn1_table_address;
    if (ppn1_table[ppn1_offset] & 0x1 == 0) {
        u64 new_page_address = kernel_pager_acquire();
        new_page_address = new_page_address / 4096;
        ppn1_table[ppn1_offset] = 1;
        ppn1_table[ppn1_offset] |= new_page_address << 10;
    }

    u64 ppn0_table_address = ((ppn1_table[ppn1_offset] >> 10) & (BIT(27) - 1)) * 4096;
    u64 *ppn0_table = (u64 *)ppn0_table_address;

    //can use to handle if already in use
    //if (ppn0_table[ppn0_offset] & 0x1 == 0) {
    //
    //}

    ppn0_table[ppn0_offset] = ((physical_address / 4096) << 10) | argument_flags | VMA_VALID;

    asm volatile ("sfence.vma %0, %1" :: "r"(virtual_address), "r"(process_asid) : "memory"); //push cache update, only to current asid
}

void arch_virtual_memory_assign_kernel_page(u64 root_table, u64 process_cache_number, u64 virtual_address, u64 physical_address, u64 argument_flags) {
    arch_virtual_memory_assign_page(root_table, process_cache_number, virtual_address, physical_address, argument_flags);
}

void arch_virtual_memory_assign_user_page(u64 root_table, u64 process_cache_number, u64 virtual_address, u64 physical_address, u64 argument_flags) {
    arch_virtual_memory_assign_page(root_table, process_cache_number, virtual_address, physical_address, argument_flags | VMA_USER);
}

void arch_virtual_memory_change_table(process_info process) {

    u64 satp_value = 0
    | (((u64)process.virtual_memory_number) << 44) //ASID ID
    | 8UL << 60 // says its sv39
    | (((u64)process.virtual_memory_root_table) / 4096);

    asm volatile ("csrw satp, %0" :: "r"(satp_value) : "memory");

    //might need fence here need to look more into it
}