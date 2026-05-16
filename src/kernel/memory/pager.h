#ifndef KERNEL_PAGER_H
#define KERNEL_PAGER_H

void kernel_pager_init();
u64 kernel_pager_acquire();
void kernel_pager_release(u64 location);

#endif