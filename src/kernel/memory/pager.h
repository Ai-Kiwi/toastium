#ifndef KERNEL_PAGER_H
#define KERNEL_PAGER_H

void kernel_pager_init();
volatile char *kernel_pager_acquire();
void kernel_pager_release(char *page_location);

#endif