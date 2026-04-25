#ifndef BOARD_H
#define BOARD_H

//visionfive2 lite
//# //also change start in linker

#define BOARD_TARGET_QEMU  0
#define BOARD_TARGET_VISIONFIVE2_LITE 1

#if BOARD_TARGET == BOARD_TARGET_QEMU
    #define UART_BASE 0x10000000
    #define ram_start_location 0x80200000
    #elif BOARD_TARGET == BOARD_TARGET_VISIONFIVE2_LITE
    #define UART_BASE 0x10000000
    #define ram_start_location 0x40200000
#else
    #error "Unknown BOARD TARGET"
#endif

//c only stuff so linker works
#ifndef LINKER_SCRIPT
#ifndef __ASSEMBLER__
    #ifdef BOARD_TARGET == BOARD_TARGET_QEMU
        typedef unsigned char  uart_reg_t;
        #elif BOARD_TARGET == BOARD_TARGET_VISIONFIVE2_LITE
        typedef unsigned int uart_reg_t;
    #else
        #error "Unknown BOARD TARGET"
    #endif
#endif 
#endif 

//#define uart_ram_location 0x80000000 //currrently unused

#endif