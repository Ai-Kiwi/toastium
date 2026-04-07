void kernel_main() {
    volatile char* uart = (volatile char*)0x10000000;
    
    char* msg = "Hello from C!\n";
    while (*msg) {
        *uart = *msg++;
    }
    
    while(1) {}
}



//todo
//interrupt handler
//virtual memory
//program schedular
//syscall
//framebuffer
//vertio disk handler
//fat32 disk reader

