#include "arch_main.h"
#include "arch_device_tree/dtb.h"
#include "include/types.h"
#include "kernel/main.h"

extern u8 _kernel_end;

void arch_main(u64 hart_id, u64 dtb_addr) {
    dtb_set_dst((u8 *)dtb_addr);

    kernel_main();
}
