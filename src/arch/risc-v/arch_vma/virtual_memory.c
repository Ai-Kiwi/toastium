//following code assumes sv39. This os realistically isn't going past that.
//everything in here is self contained assumptions, if in future more is needed
//this file can be swapped out or replaced with generic middle-man.

//for this implication the LSB for RSW field meanins non shrinkable. 
//an example use case for this is a file and you want to see if its been used.
//if it had merged it would be one large read field instead of more then 1

#include "drivers/uart/uart.h"
#include "kernel/memory/list.h"
#include "types.h"
#include "def.h"
#include "kernel/memory/pager.h"
#include "kernel/safety/panic.h"
#include "kernel/process/process.h"
#include "virtual_memory.h"
#include "board.h"

static inline u64 create_leaf(u64 phys_addr, u64 arg_flags) {
    if (arg_flags == 0x0) {
        return 0;
    }
    return ((((phys_addr - KERNEL_VMA_START) / 4096) << 10) | arg_flags | VMA_VALID);
}

static inline u64 create_branch(u64 branch_addr) {
    return (((branch_addr - KERNEL_VMA_START) / 4096) << 10) | VMA_VALID;
}

static inline u64 get_branch_loc(u64 leaf_data) {
    if ((leaf_data & ((BIT(10)) - 1)) != 0x1) {
        return 0;
    }
    return ((((leaf_data >> 10) & (BIT(44) - 1)) * 4096) + KERNEL_VMA_START);
}

static inline u64 get_leaf_flags(u64 leaf_data) {
    return (BIT(10) - 1) & (leaf_data);
}

static inline u64 get_phys_leaf_loc(u64 leaf_data) {
    if ((leaf_data & (BIT(10) - 1)) <= 0x1) { //must have flags and be valid
        return 0;
    }
    return ((((leaf_data >> 10) & (BIT(44) - 1)) * 4096) + KERNEL_VMA_START);
}

static u64 expand_leaf(u64 *leaf, u64 branch_jmp_size) {
    u64 *new_branch = (u64 *)pg_alloc();
    u64 physical_location = get_phys_leaf_loc(*leaf);
    u64 flags = get_leaf_flags(*leaf);

    if (flags & VMA_DONT_COMPRESS) {
        PANIC("ATTEMPT_TO_EXPAND_NON_COMPRESS_LEAF", 0, 0, 0);
    }

    *leaf = create_branch((u64)new_branch);

    if ((*leaf) == 0x0) {
        return (u64)new_branch; //was unmapped so just return unmapped
    }

    for (u64 i=0; i<512; i++) {
        new_branch[i] = create_leaf(physical_location + (i * (branch_jmp_size/512)), flags);
    }


    return (u64)new_branch;
}

static void destroy_branch(u64 *branch_loc, u64 branch_jmp_size) {
    volatile u64 *physical_loc = (volatile u64 *)get_branch_loc(*branch_loc);
    if ((u64)physical_loc == 0x0) {
        return;
    }

    for (u64 i=0; i<512; i++) {
        destroy_branch((u64 *)physical_loc[i], branch_jmp_size / 512);
    }

    pg_free((u64)physical_loc);
}

static void shrink_branch(u64 *branch_loc, u64 branch_jmp_size) {
    bool8 can_shrink = TRUE;
    u64 *branch_leafs = (u64 *)get_branch_loc(*branch_loc);
    if ((u64)branch_leafs == 0x0) {
        return;
    }

    u64 leaf_jmp_size = branch_jmp_size / 512;
    if (branch_leafs[0] == 0) {//unmapped
        for (u64 i=0; i<512; i++) {
            can_shrink = can_shrink && (branch_leafs[i] == 0);
        }
    }else{
        u64 start_phys_loc = get_phys_leaf_loc(branch_leafs[0]);
        u64 start_access_flags = get_leaf_flags(branch_leafs[0]);
        for (u64 i=0; i<512; i++) {
            u64 iter_phys_loc = get_phys_leaf_loc(branch_leafs[i]);
            u64 iter_flags = get_leaf_flags(branch_leafs[i]);
            can_shrink = can_shrink && (iter_phys_loc == start_phys_loc + (leaf_jmp_size * i)) && (iter_flags == start_access_flags) && ((iter_flags & VMA_DONT_COMPRESS) == 0);
        }
    }

    if (can_shrink == TRUE) {
        if (branch_leafs[0] == 0) {//unmapped
            *branch_loc = 0;
            pg_free((u64)branch_leafs);
        }else{
            u64 start_phys_loc = get_phys_leaf_loc(branch_leafs[0]);
            u64 start_access_flags = get_leaf_flags(branch_leafs[0]);
            create_leaf(start_phys_loc, start_access_flags);
            pg_free((u64)branch_leafs);
        }
    }
}

static void vma_replace_section(u64 table_root, u64 virt_addr_start, u64 virt_addr_size, u64 phys_addr, u64 access_flags, u64 vma_addr_asid) {
    const u64 ppn0_jmp_size = 4096;
    const u64 ppn1_jmp_size = 4096*512;
    const u64 ppn2_jmp_size = 4096*512*512;

    bool8 is_kernelspace = virt_addr_start >= 0xffffffc000000000;
    u64 normal_virt_addr = virt_addr_start;
    normal_virt_addr = normal_virt_addr - ((0xffffffc000000000 - 0x4000000000) * (u64)is_kernelspace); //offsets in a way where it starts from 256 instead of from 0

    if (virt_addr_size > ppn2_jmp_size * 256) {
        PANIC("VMA_MAPPING_LARGER_THEN_MAX_SIZE", virt_addr_size, 0, 0);
    }
    if (normal_virt_addr > 0x4000000000 - virt_addr_size && is_kernelspace == FALSE) { //spilling past max userspace
        PANIC("VMA_MAPPING_PAST_MAX_USERSPACE", virt_addr_start, virt_addr_size, phys_addr);
    }
    if (normal_virt_addr > 0x8000000000 - virt_addr_size && is_kernelspace == TRUE) { //spilling past max userspace
        PANIC("VMA_MAPPING_PAST_MAX_KERNELSPACE", virt_addr_start, virt_addr_size, phys_addr);
    }

    u64 mapping_left = virt_addr_size;
    u64 cur_virt_addr = normal_virt_addr;

    u64 pre_ppn0_cnt = 0;
    u64 pre_ppn1_cnt = 0;
    u64 ppn2_cnt = 0;
    u64 post_ppn1_cnt = 0;
    u64 post_ppn0_cnt = 0;

    u8 highest_reached_ppn = 0;

    while (mapping_left > 0) {
        if (ROUND_MOD_DOWN(cur_virt_addr, ppn2_jmp_size) == cur_virt_addr && highest_reached_ppn < 2) {
            highest_reached_ppn = 2;
        }
        if (ROUND_MOD_DOWN(cur_virt_addr, ppn2_jmp_size) == cur_virt_addr && highest_reached_ppn < 1) {
            highest_reached_ppn = 1;
        }

        if (ROUND_MOD_DOWN(cur_virt_addr, ppn2_jmp_size) == cur_virt_addr && mapping_left >= ppn2_jmp_size) {
            ppn2_cnt += 1;
            mapping_left -= ppn2_jmp_size;
            cur_virt_addr += ppn2_jmp_size;
        }else if (ROUND_MOD_DOWN(cur_virt_addr, ppn1_jmp_size) == cur_virt_addr && mapping_left >= ppn1_jmp_size) {
            if (highest_reached_ppn > 1){
                post_ppn1_cnt += 1;
            }else{
                pre_ppn1_cnt += 1;
            }
            mapping_left -= ppn1_jmp_size;
            cur_virt_addr += ppn1_jmp_size;
        }else if (ROUND_MOD_DOWN(cur_virt_addr, ppn0_jmp_size) == cur_virt_addr && mapping_left >= ppn0_jmp_size) {
            if (highest_reached_ppn > 0){
                post_ppn0_cnt += 1;
            }else{
                pre_ppn0_cnt += 1;
            }
            mapping_left -= ppn0_jmp_size;
            cur_virt_addr += ppn0_jmp_size;
        }else{
            PANIC("BAD_SIZE_LEFT_MAPPING", virt_addr_start, virt_addr_size, phys_addr);
        }
    }

    //loop over the list now

    u64 cur_offset = 0;
    u64 cur_ppn2_offset = normal_virt_addr / ppn2_jmp_size;
    u64 cur_ppn1_offset = (normal_virt_addr % ppn2_jmp_size) / ppn1_jmp_size;
    u64 cur_ppn0_offset = (normal_virt_addr % ppn1_jmp_size) / ppn0_jmp_size;



    u64 *ppn2_table = (u64 *)table_root;

    u64 *pre_ppn1_table = (u64 *)get_branch_loc(ppn2_table[cur_ppn2_offset]);
    if (((u64)pre_ppn1_table == 0x0) && (pre_ppn1_cnt > 0 || pre_ppn0_cnt > 0)) {
        pre_ppn1_table = (u64 *)expand_leaf(&ppn2_table[cur_ppn2_offset], ppn2_jmp_size);
    }
    u64 *pre_ppn0_table = (u64 *)get_branch_loc(pre_ppn1_table[cur_ppn1_offset]);
    if (((u64)pre_ppn0_table == 0x0) && (pre_ppn0_cnt > 0)) {
        pre_ppn0_table = (u64 *)expand_leaf(&pre_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
    }

    //pre ppn 0
    for (u64 i=0; i<pre_ppn0_cnt; i++) {
        pre_ppn0_table[cur_ppn0_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn0_jmp_size;
        cur_ppn0_offset++;
    }
    cur_ppn0_offset = 0;
    shrink_branch(&pre_ppn1_table[cur_ppn1_offset],ppn1_jmp_size);

    //pre ppn 1
    for (u64 i=0; i<pre_ppn1_cnt; i++) {
        if (get_branch_loc(pre_ppn1_table[cur_ppn1_offset]) != 0) {
            destroy_branch((u64 *)pre_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
        }
        pre_ppn1_table[cur_ppn1_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn1_jmp_size;
        cur_ppn1_offset++;
    }
    cur_ppn1_offset = 0;
    shrink_branch(&ppn2_table[cur_ppn2_offset],ppn2_jmp_size);

    //ppn 2
    for (u64 i=0; i<ppn2_cnt; i++) {
        if (get_branch_loc(ppn2_table[cur_ppn2_offset]) != 0) {
            destroy_branch((u64 *)ppn2_table[cur_ppn2_offset], ppn2_jmp_size);
        }
        ppn2_table[cur_ppn2_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn2_jmp_size;
        cur_ppn2_offset++;
    }

    //post ppn 1
    u64 *post_ppn1_table = (u64 *)get_branch_loc(ppn2_table[cur_ppn2_offset]);
    if (((u64)post_ppn1_table == 0x0) && (post_ppn1_cnt > 0 || post_ppn0_cnt > 0)) {
        post_ppn1_table = (u64 *)expand_leaf(&ppn2_table[cur_ppn2_offset], ppn2_jmp_size);
    }

    for (u64 i=0; i<post_ppn1_cnt; i++) {
        if (get_branch_loc(post_ppn1_table[cur_ppn1_offset]) != 0) {
            destroy_branch((u64 *)post_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
        }
        post_ppn1_table[cur_ppn1_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn1_jmp_size;
        cur_ppn1_offset++;
    }

    //post ppn 0
    if (post_ppn0_cnt > 0 ) {
        u64 *post_ppn0_table = (u64 *)get_branch_loc(post_ppn1_table[cur_ppn1_offset]);
        if (((u64)post_ppn0_table == 0x0) && (post_ppn0_cnt > 0)) {
            post_ppn0_table = (u64 *)expand_leaf(&post_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
        }

        for (u64 i=0; i<post_ppn0_cnt; i++) {
            post_ppn0_table[cur_ppn0_offset] = create_leaf(phys_addr + cur_offset, access_flags);
            cur_offset += ppn0_jmp_size;
            cur_ppn0_offset++;
        }
        shrink_branch(&post_ppn1_table[cur_ppn1_offset],ppn1_jmp_size);
    }

    shrink_branch(&ppn2_table[cur_ppn2_offset],ppn2_jmp_size); //shrink afterwards as it has to remain expanded for ppn 0 to use

    //loop over again but with asid updates now
    cur_offset = 0;

    for (u64 i = 0; i<pre_ppn0_cnt; i++) {
        asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr_start + cur_offset), "r"(vma_addr_asid) : "memory");
        cur_offset += ppn0_jmp_size;
    }
    for (u64 i = 0; i<pre_ppn1_cnt; i++) {
        asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr_start + cur_offset), "r"(vma_addr_asid) : "memory");
        cur_offset += ppn1_jmp_size;
    }
    for (u64 i = 0; i<ppn2_cnt; i++) {
        asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr_start + cur_offset), "r"(vma_addr_asid) : "memory");
        cur_offset += ppn2_jmp_size;
    }
    for (u64 i = 0; i<post_ppn1_cnt; i++) {
        asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr_start + cur_offset), "r"(vma_addr_asid) : "memory");
        cur_offset += ppn1_jmp_size;
    }
    for (u64 i = 0; i<post_ppn0_cnt; i++) {
        asm volatile ("sfence.vma %0, %1" :: "r"(virt_addr_start + cur_offset), "r"(vma_addr_asid) : "memory");
        cur_offset += ppn0_jmp_size;
    }


}

void vma_map_kernel(process *proc, u64 virt_addr, u64 size, u64 phys_addr, u64 access_flags) {
    vma_replace_section((u64)proc->vma_table, virt_addr, size, phys_addr, access_flags | VMA_VALID, proc->vma_addr_space_id);
}

void vma_map_user(process *proc, u64 virt_addr, u64 size, u64 phys_addr, u64 access_flags) {
    vma_replace_section((u64)proc->vma_table, virt_addr, size, phys_addr, access_flags | VMA_VALID | VMA_USER, proc->vma_addr_space_id);
}

void vma_unmap(process *proc, u64 virt_addr, u64 size) {
    vma_replace_section((u64)proc->vma_table, virt_addr, size, 0, 0, proc->vma_addr_space_id);
}











//not 64 bytes aligned for multicore
u64 max_asid;
u64 current_highest_asid;

//need to sense if it is smaller then current and expand out
//if it is same as current then compress

//to keep it simple would just look at the one level and if it is all the same rules.
//if that one section is all the same rules then that page can be wiped and replaced with a higher level one.

//one of the reserved flag will be a special bit. It basiclly tells that this part can't be compressed. It would be used for files and what not

void vma_create(process *proc) {
    //likely todo will be vma generic enough so that it isnt just processes, will keep this new generic as not shared. Then after that will use that to create the inital kernel level.
    //for kernel level it will go over a premade list of ranges and setup all of those. 
    //this data made will then be cached and just straght copied whever a new process is loaded in, slight changes will be made from there tho such as setting up per process kernel stack right at the end. Or assigning the process trapframe
    u64 new_page_addr = pg_alloc();

    //create kernel mapping
    ///allow read/write/execute, mark as global for optimization, is valid and also mark as already dirty and accessed for performance.
    const u64 access_mask = VMA_VALID | VMA_READ | VMA_WRITE | VMA_EXEC; //missing dirt and global
    volatile u64 *entry_leaf = (u64 *)new_page_addr;
    for (u64 i=0; i<256; i++) {
        entry_leaf[256 + i] = (i << 28) | access_mask;
    }

    proc->vma_table = (u64 *)new_page_addr;
}

//must be called when init vma has already been set
//Reason is it detects max vma
void vma_init() {
    //find max ASID
    asm volatile ("csrr %0, satp" : "=r"(max_asid));
    max_asid = (max_asid >> 44) & (BIT(16)-1);
    //set to max so first loop resets all
    current_highest_asid = max_asid;
}

static void vma_reset_asid() {
    //loop over all processes, set asid to -1 meaning not set.
    current_highest_asid = 0;
    asm volatile ("sfence.vma zero, zero" ::: "memory");

    if (max_asid == 0) {
        return;
    }

    list_iter iter;
    processes_iter(&iter);
    while (iter.cur_upto<iter.cur_list->item_cnt) {
        process *proc = (process *)list_iter_next(&iter);
        if (proc == 0x0) {
            PANIC("PROC_ASID_LOOP_INVALID", 0, 0, 0);
        }
        proc->vma_addr_space_id = U64_MAX;
    }
}

static u64 vma_fetch_asid() {
    if (current_highest_asid >= max_asid || max_asid == 0) {
        vma_reset_asid();
        current_highest_asid = 0;
    }else{
        current_highest_asid++;
    }
    return current_highest_asid;
}

void vma_swap(process *process) {
    bool8 new_vma = FALSE;
    if (process->vma_addr_space_id == U64_MAX || max_asid == 0) {
        process->vma_addr_space_id = vma_fetch_asid();
        new_vma = TRUE;
    }

    u64 satp_value = 0
    | (((u64)process->vma_addr_space_id) << 44) //ASID ID
    | (8UL << 60) // says its sv39
    | ((((u64)process->vma_table) - KERNEL_VMA_START) / 4096);

    asm volatile ("fence rw, rw");
    asm volatile ("fence.i");
    asm volatile ("csrw satp, %0" :: "r"(satp_value) : "memory");
    //might need fence here need to look more into it
    if (new_vma) {
        asm volatile ("sfence.vma zero, %0" :: "r"(process->vma_addr_space_id) : "memory");
    }
}

void vma_enable_read_user() {
    asm volatile("csrs sstatus, %0" :: "r"BIT(18));
}

void vma_disable_read_user() {
    asm volatile("csrc sstatus, %0" :: "r"BIT(18));
}