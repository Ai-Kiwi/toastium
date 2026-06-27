#ifndef KERNEL_HANDLES_H
#define KERNEL_HANDLES_H

#include "include/types.h"

typedef u64 kernel_handle;

typedef enum {
    KHANDLE_TYPE_FILE,
    //KHANDLE_TYPE_PIPE,
    //KHANDLE_TYPE_SOCKET,
} kernel_handle_type;


#endif