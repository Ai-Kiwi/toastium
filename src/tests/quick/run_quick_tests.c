
#include "tests/utils.h"
#include "types.h"

void tests_run_quick() {
#ifndef QUICK_TEST_MODE
    return;
#endif
    // test dtb, basic make sure items exist
    // test allocator, basic giving items
    // test file system
    // read, write, new file, delete file
    // test vma
    // test uart
    // test list
    // test pager
    // test radix
    // test string functions
    // test processes
    // test elf process
    // test schedular
    // syscall
    // timer
    //
    tests_hang(TRUE);
}

// test trap