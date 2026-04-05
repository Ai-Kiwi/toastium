ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text : {
        *(.text.boot)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        _bss_bottom = .;
        *(.bss*)
        _bss_top = .;
    }

    /* Sets up stack */
    _stack_bottom = .;
    . += 4096;
    _stack_top = .;
}