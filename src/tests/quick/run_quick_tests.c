
#include "allocator.h"
#include "pager.h"
#include "tests/utils.h"
#include "types.h"

void tests_run_quick() {
#ifndef QUICK_TEST_MODE
    return;
#endif
    // test dtb, basic make sure items exist

    test_quick_pager();

    test_quick_allocator();
    // add test for when overlapping (likely just fuzzer at that point through)

    // test vma
    // asign region, make sure value same
    // assign region twice for same location
    // doubnle assign location
    // assign large region in 1 go, make sure done bulk somehow
    // asign and make sure it auto chunked up on different asign

    // test schedular
    // add process
    // make sure process returns next
    // see ordering make sense
    // make sure not return same twice

    // test list
    // keep adding and loop over make sure valid
    // try removes around the location
    // try adds

    // test hashmap
    // make sure not to many items
    // try duplicates

    // test radix
    // make sure it gets next item correct with mapping
    // test on insert already exist

    // test string functions
    // run over all strings and give it a shot with them

    // test processes
    // make a process and make sure data is correct for it?
    // maybe try runnig have to see how stuff ends up

    // test file system
    // read
    // write
    // create
    // delete
    // delete with open file
    // multi handles
    // create folder
    // delete folder
    // delete folder with files

    // test elf process
    // start program and make sure it works cirrectly
    // will have dll stuff later

    // more complex //

    // test trap

    tests_hang(TRUE);
}
