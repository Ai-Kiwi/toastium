#ifndef DEF_H
#define DEF_H

#define BIT(bit_num) (1UL << (bit_num))

//max running processes. Both these cost to be large so kept as small as can be. Both start at 0
#define max_process_cnt 255 //large cost being larger, has to resume slots for each process internally
#define max_pid_num 255

#define KERNEL_PAGE_SIZE 4096

#define null_program_pid -1

#endif