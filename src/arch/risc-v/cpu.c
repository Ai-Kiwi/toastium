void freeze_system() {
    asm volatile ("csrc sstatus, %0" :: "r"(1 << 1)); //disable traps
    while (1){
        asm volatile("wfi");
    }
}