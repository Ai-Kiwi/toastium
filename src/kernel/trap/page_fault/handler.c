#include "kernel/trap/handler.h"
#include "handler.h"
#include "include/types.h"

bool8 pgfault_load(trap_data *trap_data) {
    // stub for now. Would attempt to load the page. Returns if it failed or
    // loaded successfully.

    return FALSE;
}
