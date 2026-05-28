#ifndef KERNEL_PAGER_H
#define KERNEL_PAGER_H

typedef struct {
    u64 page_start;
    u64 page_size;
} page_range;

void kernel_pager_init();
u64 kernel_pager_acquire();
void kernel_pager_release(u64 location);

#endif