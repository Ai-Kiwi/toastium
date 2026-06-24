misc
 - asid isn't best handled for when its not supported on hardware
 - allocator no cache system for last free
 - pager should have double past location buffer
 - pager can't handle overlap or same memory locations. Both for reserved and free
 - dtb is hard coded in a lot of locations. Will cause issues when switching between arch and is messy right now for different systems
 - bitmaps overlap with page size meaning data will be passed to user in virtual memory
 - No memory overlap handling for pager memory regions
 - No read/write/execute guard on virtual memory regions for kernelspace
 - if there is only 1 process available the schuadler can't see it for next process so won't repick it again. Meaning half is wasted cpu cycles.
 ^ Also for planing on wfi this will cause a lot of issues as won't likely have timer done.
 - the idle process doesn't use wfi

Need large pages
 - kernel stack for process is 1 page, temp fix for now will defo need to be larger very soon.
 - virtual memory assigns page by page not in bulk
 - virtual memory doesn't support mega or giga pages
 - Allocator should use multipages when support is added
 - pager should support multipages
 - user processes don't do large pages

Need bss fix
 - asid upto data not aligned 64 bytes (current and max)
 - processes info
 - current running process info
 - process created up to id
 - Allocator data

Red black tree
 - scheduler
 - timer
