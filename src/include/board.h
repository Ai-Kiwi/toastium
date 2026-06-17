#ifndef BOARD_H
#define BOARD_H

//visionfive2 lite
//# //also change start in linker

// 1024*1024*1024*512 = 0x4000000000 (half of sv39 space for risc-v)
//0x4000000000 would be jump but top half is top half
#define KERNEL_VMA_START 0xffffffc000000000
#define TRAPFRAME_ADDRESS 0x0000003FFFFFF00
//16kb stack size for each hart
#define HART_KERNEL_STACK_SIZE 4096 * 4

#define BOARD_TARGET_QEMU  0
#define BOARD_TARGET_VISIONFIVE2_LITE 1

#if BOARD_TARGET == BOARD_TARGET_QEMU
    #define UART_BASE 0x10000000
 #elif BOARD_TARGET == BOARD_TARGET_VISIONFIVE2_LITE
    #define UART_BASE 0x10000000
#else
    #error "Unknown BOARD TARGET"
#endif

#ifndef GIT_VERSION_HASH
    #define COMPILE_VERSION "Unknown"
#else
    #define COMPILE_VERSION GIT_VERSION_HASH
#endif

//c only stuff so linker works
#ifndef LINKER_SCRIPT
#ifndef __ASSEMBLER__
    #include "types.h"
    #ifdef BOARD_TARGET == BOARD_TARGET_QEMU
        typedef u8  uart_reg_t;
    #elif BOARD_TARGET == BOARD_TARGET_VISIONFIVE2_LITE
        typedef u32 uart_reg_t;
    #else
        #error "Unknown BOARD TARGET"
    #endif
#endif
#endif

//#define uart_ram_location 0x80000000 //currrently unused

#endif