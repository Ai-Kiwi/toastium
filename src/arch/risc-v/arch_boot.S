#include "board.h"
#include "arch_trap/state.S"
.section .text.boot
.global _start

//I keep neededing this and forgetting how todo it so for future me and anyone reading lol
//li   a0, 0x10000000
//li   a1, 'A'
//sb   a1, 0(t0)

_start:
    .option norvc
    j setup_vma //code 1
    .option rvc
    .long 0 // code 2
    .quad 0x200000 // image load offset (little endian)
    .quad _kernel_end - _kernel_start //image size, little endian
    .quad 0 //kernel flags, little endian
    .long 2 //version of header
    .long 0 //reserved
    .quad 0 //reserved
    .quad 0x5643534952 //magic meaning "RISCV", little endian
    .long 0x05435352 //magic 2 meaning "RSC\x05", little endian
    .long 0 //PE COFF offset

setup_vma:
    //bind for traps
    la t0, trap_entry
    csrw stvec, t0

    //setup stack top
    la t0, _stack_top
    addi sp, t0, 0

    //setup virtual memory maps
    la t0, _kernel_vma_offset_table //location for vma table
    li t6, 4096
    div t0, t0, t6
    mul t0, t0, t6 //rounds it down

    li t6, 2048 // 2048 stored

    addi t1, zero, 0 //loop num

    j setup_vmn_table

setup_vmn_table:
    //t0 : vma location
    //t1 : loop num (bytes)
    //t2 : loop num
    //t3 : output location
    //t4 : page entry
    //t5 : PPN DATA
    //t6 : 2048

    bgeu t1, t6, finish_vma_setup
    li t2, 8
    div t2, t1, t2

    //setup kernel address range
    li t4, 0xEF //allow read/write/execute, mark as global for optimization, is valid and also mark as already dirty and accessed for performance .

    slli t5, t2, 28 //move the current iter into PPN[2] (located at bit 28)

    add t4, t4, t5 //add together, final page entry

    //get location
    add t3, t1, t0
    add t3, t3, t6

    sd t4, 0(t3) //write out

    //setup userspace, for now kernel until jump then blank
    li t4, 0xCF //allow read/write/execute, is valid and also mark as already dirty and accessed for performance .

    add t4, t4, t5 ///add together, final page entry

    //get location
    add t3, t1, t0

    sd t4, 0(t3) //write out

    addi t1, t1, 8
    j setup_vmn_table

finish_vma_setup:

    //t0 is base vma table location
    //ASID needs to be highest num as this will be reserved for kernel process
    la t1, _kernel_vma_offset_table
    li t2, 4096
    div t1, t1, t2

    li t3, 0xFFFF // ASID id, max possible
    slli t3, t3, 44

    add t1, t1, t3 // Add ASID value

    li t3, 8 // type value (sv39)
    slli t3, t3, 60
    add t1, t1, t3 // Add type value

    //apply table location and info
    csrw satp, t1

    fence.i

    //flush TLB so it applies changes which we have made (not satp but VMA table)
    sfence.vma zero, zero

    la t3, jump_blank_first_half_vma_table
    li t2, KERNEL_VMA_START
    add t3, t3, t2
    jalr zero, 0(t3) //make the jump to real location


jump_blank_first_half_vma_table:
    la t0, _kernel_vma_offset_table
    j blank_first_half_vma_table

blank_first_half_vma_table:
    bgeu t1, t6, real_start
    add t4, zero, zero //final value

    add t3, t1, t0
    sd t4, 0(t3) //output

    addi t1, t1, 8
    j blank_first_half_vma_table

real_start:

    #flush TLB so that lower half doesn't contain kernel code anymore
    sfence.vma zero, zero

    fence.i

    //setup stack top
    la t0, _stack_top
    addi sp, t0, 0

    //bind for new interrupts
    la t0, trap_entry
    csrw stvec, t0

    //clear out the bss
    la t0, _bss_bottom
    la t1, _bss_top
    j clear_bss

clear_bss:
    bgeu t0, t1, start_os
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss

start_os:
    call arch_main

    #load in data for next process and run it
    LOAD_TRAP_FRAME
    sret