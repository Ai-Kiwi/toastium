misc
 - asid isn't best handled for when its not supported on hardware
 - allocator no cache system for last free
 - pager should have double past location buffer
 - pager can't handle overlap or same memory locations. Both for reserved and free
 - dtb is hard coded in a lot of locations. Will cause issues when switching between arch and is messy right now for different systems
 - bitmaps overlap with page size meaning data will be passed to user in virtual memory
 - No memory overlap handling for pager memory regions
 - if there is only 1 process available the schuadler can't see it for next process so won't repick it again. Meaning half is wasted cpu cycles.
 ^ Also for planing on wfi this will cause a lot of issues as won't likely have timer done.
 - the idle process doesn't use wfi
 - floating point reg don't have state saved or swapped for context shift. 
 - read from userspace reads byte at a time not 64bits at a time. (would need to keep align in mind too) 
 - hashmap for processes should be expanded in size or auto size. (rn only a single page)
 - kernel vma no longer has global args for kernel side
 - No read/write/execute guard on virtual memory regions for kernelspace
 - No good trap handling if before context switch as nothing is loaded for a trapframe.
 - IRQ should enable after context switch, currently panics likely guess something queued and bug in trap system. (this doesn't completely make sense tho so will have to investigate)

Need large pages
 - kernel stack for process is 1 page, temp fix for now will defo need to be larger very soon.
 - virtual memory assigns page by page not in bulk
 - virtual memory doesn't support mega or giga pages
 - Allocator should use multipages when support is added
 - pager should support multipages
 - user processes don't do large pages
 - pipe/sockets need merged pages

Need bss fix
 - asid upto data not aligned 64 bytes (current and max)
 - processes info
 - current running process info

needs to use red black tree
 - children of file descriptor (currently list)
 - scheduler (currently list with fifo)
 - timer (currently nothing) 
 - driver manager for loading drivers. Uses linked list right now.

hashmap
 - hashmap has 64byte per item overhead which is not needed (for cache coherency). Possible fix is todo sub lists to fix this.
 - needs to handle overwrite for insert. Instead of just being unhandled, it should overwrite old value with a new one.
 - needs to use random system for hashes to stop attack vector.

test suits
 - pager tests to make sure it allocates again in order.
 - allocator needs more through testing to make sure it reallocates in order right
 - allocator currently has unfixed bug
 - hashmap needs system to make sure it doesn't remap to same location repetitively.
 - hashmap needs remove item testing.
 - list system
 - vma system
 - scheduler
 - trap handler

Needs allocator combining (combine more then 1 entry into 64byte chunks)
 - hashmap entry
 - red black tree entry (will be when coded)

hard fix for dtb address-cell and cell size as 64bit ("safely" panics)
 - assumed in pager

VFS
 - read and write are byte by byte not 64bits at a time

High priority.
 - There is a bug with allocator todo with not reusing sections. Issue appears in 
 - hashmap needs to remove old value on insert and return it