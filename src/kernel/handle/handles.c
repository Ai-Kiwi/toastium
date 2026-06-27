#include "include/types.h"
#include "kernel/process/process.h"
#include "kernel/safety/panic.h"
#include "kernel/handle/handles.h"

u64 kernel_handle_new_blank(kernel_process *process, kernel_handle_type type) {
    switch (type) {
    case KHANDLE_TYPE_FILE:
      break;
    default:
      PANIC("ATTEMPT_OPEN_INVALID_HANDLE_TYPE", (u64)type, 0, 0);
    }
    return 0;
}

u64 kernel_handle_fetch() {
    //fetch item via hashmap using key
    //read first 4 bytes and figure out which handle type it is and use appropriate

    return 0;
}

void kernel_handle_remove() {

}
