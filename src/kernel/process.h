#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

void init_processes();

typedef struct {
    pid process_id;
    //some form of info on what its waiting for
    bool8 alive;
} kernel_process_info;

#endif