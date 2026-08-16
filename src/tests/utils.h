#ifndef TESTS_UTILS_H
#define TESTS_UTILS_H

#include "include/types.h"

void asset_u64(u64 first_value, u64 second_value);
void test_print_step(char *text, u64 cur_step, u64 finish_step, u64 step_size);
void test_print_next();
void tests_hang();

#endif
