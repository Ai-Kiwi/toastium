#ifndef KERNEL_PAGER_H
#define KERNEL_PAGER_H

#include "types.h"

typedef struct {
    u64 page_start;
    u64 page_size;
} page_range;

void pager_init();
u64 pg_alloc();
void pg_free(u64 location);

#endif