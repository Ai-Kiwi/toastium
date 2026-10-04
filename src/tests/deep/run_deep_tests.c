#include "tests/deep/allocator.h"
#include "tests/deep/hashmap.h"
#include "tests/deep/pager.h"
#include "tests/deep/radix.h"
#include "tests/utils.h"

void tests_run_deep() {
#ifndef DEEP_TEST_MODE
    return;
#endif

    test_pager();
    test_allocator();
    test_radix();
    test_hashmap();
    tests_hang(TRUE);
}