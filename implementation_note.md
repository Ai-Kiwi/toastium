# List of poorly coded things/things to change
 - Gigabyte pages for kernel data
 - No read/write/execute guard on virtual memory regions for kernelspace
 - No memory overlap handling for pager memory regions
 - bitmaps overlap with page size meaning data will be passed to user in virtual memory
 - dtb is hard coded in a lot of locations. Will cause issues when switching between arch and is messy right now for different systems
 - pager can't handle overlap or same memory locations. Both for reserved and free
 - Allocator should be changed to store the header data once at top of page data instead of per entry
 - pager should support multipages
 - Allocator should use multipages when support is added
 - pager should have double past location buffer
 - allocator no cache system for last free
 - virtual memory assigns page by page not in bulk
 - virtual memory doesn't support mega or giga pages
 - allocator should be 64byte aligned for cross core. (Mostly header issue, per page and per entry)
 - asid isn't best handled for when its not supported on hardware

Need bss fix
 - asid not aligned 64 bytes (current and max)
 - processes info
 - current running process info