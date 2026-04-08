void freeze_system() {
    asm volatile("csrci sstatus, 8"); //disable traps
    while (1){
        asm("wfi");
    }
}