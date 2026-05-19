#ifndef DEF_H
#define DEF_H

#define BIT(bit_number) (1UL << (bit_number))

//max running processes. Both these cost to be large so kept as small as can be. Both start at 0
#define max_process_count 255 //large cost being larger, has to resume slots for each process internally
#define max_pid_number 255

#define KERNEL_PAGE_SIZE 4096

#define null_program_pid -1

#endif