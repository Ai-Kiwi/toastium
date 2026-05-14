#ifndef KERNEL_PAGER_H
#define KERNEL_PAGER_H

void kernel_pager_init();
unsigned long kernel_pager_acquire();
void kernel_pager_release(unsigned long location);

#endif