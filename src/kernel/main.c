void kernel_main() {
    volatile char* uart = (volatile char*)0x10000000;
    
    char* msg = "Hello from C!\n";
    while (*msg) {
        *uart = *msg++;
    }
    
    while(1) {}
}

//will have a static array for each update, e.g disk and uart. Drivers can add to this then from there kernel will loop over it and do whatever with it. This way drivers never talk to services and what not.





//todo
//interrupt handler
//virtual memory
//program schedular
//syscall
//framebuffer
//vertio disk handler
//fat32 disk reader
//make buffers for updates auto resize instead of fixed size
