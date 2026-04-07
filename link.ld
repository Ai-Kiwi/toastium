ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text : ALIGN(16) {
        *(.text.boot)
        *(.text*)
    }

    .rodata : ALIGN(16) {
        *(.rodata*)
    }

    .data : ALIGN(16) {
        *(.data*)
    }

    .bss : ALIGN(16) {
        _bss_bottom = .;
        *(.bss*)
        _bss_top = .;
    }

    /* Sets up stack */
    _stack_bottom = ALIGN(16);
    . += 4096;
    _stack_top = ALIGN(16);
}