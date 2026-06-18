#include "board.h"

ENTRY(_start)

//fair amount of stuff in here breaks with KERNEL_PAGE_SIZE changing. 
//Or not completely breaks but assumptions break.

PHDRS
{
  execute PT_LOAD FLAGS(5); /* 5 = Read (4) + Execute (1) */
  data PT_LOAD FLAGS(6); /* 6 = Read (4) + Write (2) */
}

SECTIONS
{
    .text : ALIGN(16) {
        _kernel_start = .;
        *(.text.boot)
        *(.text*)
    }:execute

    .trap : ALIGN(16) {
        *(.text.trap)
    }:execute

    .rodata : ALIGN(16) {
        *(.rodata*)
    }:execute

    . = ALIGN(4096); /*   /* Keeps into page so that permission system stops running. */

    .data : ALIGN(16) {
        *(.data*)
    }:data

    .bss : ALIGN(16) {
        _bss_bottom = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(16);
        _bss_top = .;
    }:data

    . = . + 16;



    .stack (NOLOAD) : ALIGN(16) {
        . += 8;
        _stack_bottom_test = .;
        . += 8;

        _stack_bottom = .;
        . += 0x4000;   /* 16KB stack */
        _stack_top = .;

        . += 8;
        _stack_top_test = .;
        . += 8;
    }:data

    . = . + 4096; //buffer added so when vma offset table rounded down
    //location doesn't go into data below it (stack)


    .stack (NOLOAD) : ALIGN(4096) {
        _kernel_vma_offset_table = .;
        . += 4096;
    }:data

    . = ALIGN(4096);

    .idle_process : ALIGN(4096) {
        _kernel_idle_process = .;
        *(.text.idle_process)
        . = . + 8;
    }:execute

    . = ALIGN(4096);

    _kernel_end = .;
    _hart_stacks_start = .;
}