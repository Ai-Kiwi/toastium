ENTRY(_start)

PHDRS
{
  execute PT_LOAD FLAGS(5); /* 5 = Read (4) + Execute (1) */
  data PT_LOAD FLAGS(6); /* 6 = Read (4) + Write (2) */
}

SECTIONS
{
    . = 0x80000000;

    .text : ALIGN(16) {
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
        _stack_bottom = .;
        . += 0x4000;   /* 16KB stack */
        _stack_top = .;
    }:data

    . = . + 16;

    _unused_ram_start = .;
}