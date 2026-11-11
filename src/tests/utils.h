#ifndef TESTS_UTILS_H
#define TESTS_UTILS_H

#include "include/types.h"

void assert_u64(u64 first_value, u64 second_value, const char *file,
                const s64 file_line, const char *function);

#define ASSERT_U64(first_value, second_value)                                  \
    assert_u64(first_value, second_value, __FILE__, __LINE__, __FUNCTION__);

void test_print_step(char *text, u64 cur_step, u64 finish_step, u64 step_size);
void test_print_next();
void tests_hang(bool8 success);
void test_print_stage_start(char *name);
void test_print_stage_pass();

#endif