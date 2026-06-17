#include "board.h"

.global STORE_TRAP_FRAME
.global LOAD_TRAP_FRAME


# Uses 272 so it sticks to being align to 16

.macro STORE_TRAP_FRAME
    csrw sscratch, x1
    li x1, TRAPFRAME_ADDRESS
    sd x1, 0(x1) #tf->register_1
    sd x2, 8(x1) #tf->register_2
    sd x3, 16(x1) #tf->register_3
    sd x4, 24(x1) #tf->register_4
    sd x5, 32(x1) #tf->register_5
    sd x6, 40(x1) #tf->register_6
    sd x7, 48(x1) #tf->register_7
    sd x8, 56(x1) #tf->register_8
    sd x9, 64(x1) #tf->register_9
    sd x10, 72(x1) #tf->register_10
    sd x11, 80(x1) #tf->register_11
    sd x12, 88(x1) #tf->register_12
    sd x13, 96(x1) #tf->register_13
    sd x14, 104(x1) #tf->register_14
    sd x15, 112(x1) #tf->register_15
    sd x16, 120(x1) #tf->register_16
    sd x17, 128(x1) #tf->register_17
    sd x18, 136(x1) #tf->register_18
    sd x19, 144(x1) #tf->register_19
    sd x20, 152(x1) #tf->register_20
    sd x21, 160(x1) #tf->register_21
    sd x22, 168(x1) #tf->register_22
    sd x23, 176(x1) #tf->register_23
    sd x24, 184(x1) #tf->register_24
    sd x25, 192(x1) #tf->register_25
    sd x26, 200(x1) #tf->register_26
    sd x27, 208(x1) #tf->register_27
    sd x28, 216(x1) #tf->register_28
    sd x29, 224(x1) #tf->register_29
    sd x30, 232(x1) #tf->register_30
    sd x31, 240(x1) #tf->register_31
    # Save info on reason and info on trap
    csrr a0, scause
    csrr a1, sepc
    csrr a2, stval
    csrr a3, sstatus
    sd a0, 248(x1) #tf->scause
    sd a1, 256(x1) #tf->sepc
    sd a2, 264(x1) #tf->stval
    sd a3, 272(x1) #tf->sstatus

    #now store x1
    add x2, x1, zero
    csrr x1, sscratch
    sd x1, 0(x2) #tf->register_1

    fence rw, rw
.endm

.macro LOAD_TRAP_FRAME
    li x1, TRAPFRAME_ADDRESS
    ld x2, 8(x1) #tf->register_2
    ld x3, 16(x1) #tf->register_3
    ld x4, 24(x1) #tf->register_4
    ld x5, 32(x1) #tf->register_5
    ld x6, 40(x1) #tf->register_6
    ld x7, 48(x1) #tf->register_7
    ld x8, 56(x1) #tf->register_8
    ld x9, 64(x1) #tf->register_9
    ld x10, 72(x1) #tf->register_10
    ld x11, 80(x1) #tf->register_11
    ld x12, 88(x1) #tf->register_12
    ld x13, 96(x1) #tf->register_13
    ld x14, 104(x1) #tf->register_14
    ld x15, 112(x1) #tf->register_15
    ld x16, 120(x1) #tf->register_16
    ld x17, 128(x1) #tf->register_17
    ld x18, 136(x1) #tf->register_18
    ld x19, 144(x1) #tf->register_19
    ld x20, 152(x1) #tf->register_20
    ld x21, 160(x1) #tf->register_21
    ld x22, 168(x1) #tf->register_22
    ld x23, 176(x1) #tf->register_23
    ld x24, 184(x1) #tf->register_24
    ld x25, 192(x1) #tf->register_25
    ld x26, 200(x1) #tf->register_26
    ld x27, 208(x1) #tf->register_27
    ld x28, 216(x1) #tf->register_28
    ld x29, 224(x1) #tf->register_29
    ld x30, 232(x1) #tf->register_30
    ld x31, 240(x1) #tf->register_31

    #loads in x1 last as being used as ptr right now
    ld x1, 0(x1) ##tf->register_1 
.endm
