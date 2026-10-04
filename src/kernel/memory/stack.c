#include "stack.h"
#include "def.h"
#include "kernel/memory/pager.h"
#include "layout.h"

// stores location to data contains info on where each of the hart stacks is
// currently a list for each hart, later will be single ptr per core, will need
// to find all uses and replace those when that happens. This is bad code but
// yeah
u64 *hart_stacks_list;

void stack_init(u64 hart_cnt) {
    hart_stacks_list = (u64 *)pg_alloc();
    for (u64 i = 0; i < hart_cnt * (HART_KERN_STACK_SIZE / KERNEL_PAGE_SIZE);
         i++) {
        hart_stacks_list[i] = pg_alloc();
    }
}