#include "../../drivers/uart/uart.h"
#include "panic.h"
#include "arch_irq.h"
#include "safety.h"

typedef struct {
    unsigned long register_1;
    unsigned long register_2;
    unsigned long register_3;
    unsigned long register_4;
    unsigned long register_5;
    unsigned long register_6;
    unsigned long register_7;
    unsigned long register_8;
    unsigned long register_9;
    unsigned long register_10;
    unsigned long register_11;
    unsigned long register_12;
    unsigned long register_13;
    unsigned long register_14;
    unsigned long register_15;
    unsigned long register_16;
    unsigned long register_17;
    unsigned long register_18;
    unsigned long register_19;
    unsigned long register_20;
    unsigned long register_21;
    unsigned long register_22;
    unsigned long register_23;
    unsigned long register_24;
    unsigned long register_25;
    unsigned long register_26;
    unsigned long register_27;
    unsigned long register_28;
    unsigned long register_29;
    unsigned long register_30;
    unsigned long register_31;
    unsigned long scause; //Address that caused trap
    unsigned long sepc; //Why trap happened
    unsigned long stval; //Extra trap info, e.g page fault says address in question
} trap_info;


void arch_trap_handler(trap_info *trap_data) {//will have pointer input here that points to reg data on stack
    uart_print_chars("Trap has just happened\n"); // bad idea but just here for time being while debugging
    kernel_safety_test();
    PANIC("UNHANDLED_TRAP_OCCURRED",trap_data->scause);
}