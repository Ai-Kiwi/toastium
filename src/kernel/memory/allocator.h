#ifndef KERNEL_ALLOCATOR_H
#define KERNEL_ALLOCATOR_H

void kernel_allocator_init();
unsigned long kernel_allocator_acquire(unsigned long size_bytes);
void kernel_allocator_release(unsigned long location);
unsigned long kernel_allocator_bump(unsigned long size);

#endif