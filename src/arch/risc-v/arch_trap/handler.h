#ifndef ARCH_TRAP_HANDLER_H
#define ARCH_TRAP_HANDLER_H

#include "include/types.h"
typedef struct {
    u64 register_1;
    u64 register_2;
    u64 register_3;
    u64 register_4;
    u64 register_5;
    u64 register_6;
    u64 register_7;
    u64 register_8;
    u64 register_9;
    u64 register_10;
    u64 register_11;
    u64 register_12;
    u64 register_13;
    u64 register_14;
    u64 register_15;
    u64 register_16;
    u64 register_17;
    u64 register_18;
    u64 register_19;
    u64 register_20;
    u64 register_21;
    u64 register_22;
    u64 register_23;
    u64 register_24;
    u64 register_25;
    u64 register_26;
    u64 register_27;
    u64 register_28;
    u64 register_29;
    u64 register_30;
    u64 register_31;
    u64 scause; //What caused the trap. (Trap code)
    u64 sepc; //What instruction caused trap (Instruction location)
    u64 stval; //Extra trap info, e.g page fault says address in question
    u64 sstatus; //Privilege level machine was in.
} arch_trap_state;

#endif
