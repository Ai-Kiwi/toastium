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

u64 create_leaf(u64 phys_addr, u64 arg_flags) {
    return ((phys_addr / 4096) << 10) | arg_flags | VMA_VALID;
}

u64 create_branch(u64 branch_addr) {
    return ((branch_addr / 4096) << 10) | VMA_VALID;
}

u64 get_branch_loc(u64 leaf_data) {
    if ((leaf_data & (BIT(10)) - 1) == 0x1) { //only valid is set so its a branch
        return (((leaf_data >> 10) & (BIT(27) - 1)) * 4096) + KERNEL_VMA_START;
    }else{
        return 0;
    }
}

u64 get_leaf_flags(u64 *leaf_data) {
    return (BIT(10) - 1) & (*leaf_data);
}

u64 get_phys_leaf_loc(u64 leaf_data) {
    if ((leaf_data & (BIT(10) - 1)) > 0x1) { // must be a leaf as has flags and valid
        return (((leaf_data >> 10) & (BIT(27) - 1)) * 4096);
    }else{
        return 0;
    }
}

u64 expand_leaf(u64 *leaf, u64 branch_jmp_size) {
    u64 *new_branch = (u64 *)pg_alloc();
    u64 physical_location = get_phys_leaf_loc(*leaf);
    u64 flags = get_leaf_flags(leaf);

    if ((*leaf) == 0x0) {
        return (u64)new_branch; //was unmapped so just return unmapped
    }

    for (u64 i=0; i<512; i++) {
        new_branch[i] = create_leaf(physical_location + (i * (branch_jmp_size/512)), flags);
    }

    return (u64)new_branch;
}

void destroy_branch(u64 branch_location, u64 branch_jmp_size) {
    volatile u64 *physical_loc = (volatile u64 *)get_branch_loc(branch_location);
    if ((u64)physical_loc == 0x0) {
        return;
    }

    for (u64 i=0; i<512; i++) {
        destroy_branch(physical_loc[i], branch_jmp_size / 512);
    }

    pg_free((u64)physical_loc);
}

void shrink_branch() {
    //loop over all leafs inside it and if they all follow each other directly then delete that table and replace contents with parent leaf instead
    //keeps in mind to ignore special cases
    //also should look into if it has no rwx commands as that is a ignore all as its acting as a guard
    //when i go over and recode this to support unmapping this will also need to support that
}

//needs shrink done
//needs testing done
//needs way to unmap. For unmap im thinking it will have nothing in access flags.
//probs will do a different unmap function but will just call this main one with unmap. Will have to see how clean that works out being
//needs way to support page as unmapped region

NOtes above for programming this

void vma_map(u64 table_root, u64 virt_addr_start, u64 virt_addr_size, u64 phys_addr, u64 access_flags) {
    const u64 ppn0_jmp_size = 4096;
    const u64 ppn1_jmp_size = 4096*512;
    const u64 ppn2_jmp_size = 4096*512*512;

    bool8 is_kernelspace = virt_addr_start >= 0xffffffc000000000;
    u64 normal_virt_addr = virt_addr_start;
    if (is_kernelspace) {
        normal_virt_addr = normal_virt_addr - (0xffffffc000000000 - 0x4000000000);
    }

    if (virt_addr_size > ppn2_jmp_size * 256) {
        PANIC("VMA_MAPPING_LARGER_THEN_MAX_SIZE", virt_addr_size, 0, 0);
    }
    if (U64_MAX - virt_addr_size - 1 >= virt_addr_start) { //take max value remove the size and if start is more then that its overflow
        PANIC("VMA_MAPPING_PAST_MAX_KERNEL", virt_addr_size, 0, 0);
    }
    if (virt_addr_start + virt_addr_size > 0x4000000000) { //spilling past max userspace
        PANIC("VMA_MAPPING_PAST_MAX_USERSPACE", virt_addr_size, 0, 0);
    }
    if (virt_addr_start > 0x4000000000 && virt_addr_start < 0xffffffc000000000) {
        PANIC("VMA_MAPPING_BETWEEN_USERSPACE_AND_KERNRELSPACE", virt_addr_size, 0, 0);
    }

    u64 mapping_left = virt_addr_size;
    u64 cur_virt_addr = normal_virt_addr;

    u64 pre_ppn0_cnt = 0;
    u64 pre_ppn1_cnt = 0;
    u64 ppn2_cnt = 0;
    u64 post_ppn1_cnt = 0;
    u64 post_ppn0_cnt = 0;

    u8 highest_reached_ppn = 0;

    while (TRUE) {
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

    for (u64 i=0; i<pre_ppn0_cnt; i++) {
        pre_ppn0_table[cur_ppn0_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn0_jmp_size;
        cur_ppn0_offset++;
    }
    cur_ppn0_offset = 0;


    for (u64 i=0; i<pre_ppn1_cnt; i++) {
        if (get_branch_loc(pre_ppn1_table[cur_ppn1_offset]) != 0) {
            destroy_branch(pre_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
        }
        pre_ppn1_table[cur_ppn1_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        shrink_branch();
        cur_offset += ppn1_jmp_size;
        cur_ppn1_offset++;
    }
    cur_ppn1_offset = 0;

    for (u64 i=0; i<ppn2_cnt; i++) {
        if (get_branch_loc(ppn2_table[cur_ppn2_offset]) != 0) {
            destroy_branch(ppn2_table[cur_ppn2_offset], ppn2_jmp_size);
        }
        ppn2_table[cur_ppn2_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        shrink_branch();
        cur_offset += ppn2_jmp_size;
        cur_ppn2_offset++;
    }

    u64 *post_ppn1_table = (u64 *)get_branch_loc(ppn2_table[cur_ppn2_offset]);
    if (((u64)post_ppn1_table == 0x0) && (post_ppn1_cnt > 0 || post_ppn0_cnt > 0)) {
        post_ppn1_table = (u64 *)expand_leaf(&post_ppn1_table[cur_ppn1_offset], ppn2_jmp_size);
    }

    for (u64 i=0; i<post_ppn1_cnt; i++) {
        if (get_branch_loc(post_ppn1_table[cur_ppn1_offset]) != 0) {
            destroy_branch(post_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
        }
        post_ppn1_table[cur_ppn1_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        shrink_branch();
        cur_offset += ppn1_jmp_size;
        cur_ppn1_offset++;
    }

    u64 *post_ppn0_table = (u64 *)get_branch_loc(post_ppn1_table[cur_ppn1_offset]);
    if (((u64)post_ppn0_table == 0x0) && (post_ppn0_cnt > 0)) {
        post_ppn0_table = (u64 *)expand_leaf(&post_ppn1_table[cur_ppn1_offset], ppn1_jmp_size);
    }

    for (u64 i=0; i<post_ppn0_cnt; i++) {
        post_ppn0_table[cur_ppn0_offset] = create_leaf(phys_addr + cur_offset, access_flags);
        cur_offset += ppn0_jmp_size;
        cur_ppn0_offset++;
    }

    //do asid updates now
}
















//not 64 bytes aligned for multicore
u64 max_asid;
u64 current_highest_asid;

void write_leaf(u64 leaf_loc, u64 dst_loc) {

}

void compress_branch(u64 branch_loc, u64 branch_lvl) {
    //just looks and children and sees if it can compress. If any of the children have a child then just return.
    //idea is the children will have this compress ran on them before any parent does so it fixes on that front.
}

//need to sense if it is smaller then current and expand out
//if it is same as current then compress

//to keep it simple would just look at the one level and if it is all the same rules.
//if that one section is all the same rules then that page can be wiped and replaced with a higher level one.

//one of the reserved flag will be a special bit. It basiclly tells that this part can't be compressed. It would be used for files and what not

u64 vma_create() {
    needs to setup kernel space as with rwx, after that also needs to asssign the kernel stack process to the end region with guarding around it.
    //likely todo will be vma generic enough so that it isnt just processes, will keep this new generic as not shared. Then after that will use that to create the inital kernel level.
    //for kernel level it will go over a premade list of ranges and setup all of those. 
    //this data made will then be cached and just straght copied whever a new process is loaded in, slight changes will be made from there tho such as setting up per process kernel stack right at the end. Or assigning the process trapframe
    u64 new_page_addr = pg_alloc();

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
void vma_init() {
    //find max ASID
    asm volatile ("csrr %0, satp" : "=r"(max_asid));
    max_asid = (max_asid >> 44) & (BIT(16)-1);
    //set to max so first loop resets all
    if (max_asid == 0){
        PANIC("MAX_ASID_IS_ZERO", 0, 0, 0);
    }
    current_highest_asid = max_asid;
}

void delete_process_asid(u64 process_ptr, u64 parameter) {
    process *proc = (process *)process_ptr;

    proc->vma_addr_space_id = 0;
}

void vma_reset_asid() {
    uart_println_str("reset all vma asid");
    //loop over all processes, set asid to -1 meaning not set.
    current_highest_asid = 0;
    asm volatile ("sfence.vma zero, zero" ::: "memory");

    processes_iter(delete_process_asid, 0);
}

u64 vma_fetch_asid() {
    if (current_highest_asid >= max_asid) {
        vma_reset_asid();
        current_highest_asid = 0;
    }else{
        current_highest_asid++;
    }
    return current_highest_asid;
}

void vma_assign(process *process, u64 virt_addr, u64 vma_phys_addr, u64 arg_flags) {
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
        u64 new_page_addr = pg_alloc();
        new_page_addr = (new_page_addr - KERNEL_VMA_START) / 4096;
        ppn2_table[ppn2_offset] = 1;
        ppn2_table[ppn2_offset] |= new_page_addr << 10;
    }

    u64 ppn1_table_addr = (((ppn2_table[ppn2_offset] >> 10) & (BIT(27) - 1)) * 4096) + KERNEL_VMA_START;
    u64 *ppn1_table = (u64 *)ppn1_table_addr;
    if ((ppn1_table[ppn1_offset] & 0x1) == 0) {
        u64 new_page_addr = pg_alloc();
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

void vma_assign_kernel(process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags) {
    vma_assign(process, virt_addr, phys_addr, arg_flags);
}

void vma_assign_user(process *process, u64 virt_addr, u64 phys_addr, u64 arg_flags) {
    vma_assign(process, virt_addr, phys_addr, arg_flags | VMA_USER);
}

void vma_swap(process *process) {
    if (process->vma_addr_space_id == U64_MAX) {
        process->vma_addr_space_id = vma_fetch_asid();
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

void vma_enable_read_user() {
    asm volatile("csrs sstatus, %0" :: "r"BIT(18));
}

void vma_disable_read_user() {
    asm volatile("csrc sstatus, %0" :: "r"BIT(18));
}