#ifndef DEF_H
#define DEF_H

#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define BIT(bit_num) (1UL << (bit_num))

// max running processes. Both these cost to be large so kept as small as can
// be. Both start at 0
#define max_process_cnt 255
#define max_pid_num 255

// ths is defined here but is later not used in many places
//  For this reason it is unsafe to change this.
#define KERNEL_PAGE_SIZE 4096

#define ROUND_MOD_DOWN(number, rounder) (((number) / (rounder)) * (rounder))
#define ROUND_MOD_UP(number, rounder)                                          \
    ((((number) + ((rounder) - 1)) / (rounder)) * (rounder))

#endif
