void __libc_start_main() {
    // setup libc
    // call main
    // unsetup libc
    // cleanup
    // exit to kernel again
}

void print() {
    char *string = "Hello from libc!";
    register unsigned long a0 asm("a0") = 1;
    register unsigned long a1 asm("a1") = 1;
    register unsigned long a2 asm("a2") = (unsigned long)string;
    asm("ecall"
        : "+r"(a0)                  // outputs
        : "r"(a0), "r"(a1), "r"(a2) // inputs
        : "memory"                  // says ram could have had updated values
    );
}