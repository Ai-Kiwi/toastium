# List of poorly coded things/things to change
 - Gigabyte pages for kernel data
 - No read/write/execute guard on virtual memory regions
 - No memory overlap handling for pager memory regions
 - bitmaps overlap with page size meaning data will be passed to user in virtual memory
 - Using int instead of size_t. Same for long and char.
 - dtb is hard coded in a lot of locations. Will cause issues when switching between arch and is messy right now for different systems
 - pager can't handle overlap or same memory locations. Both for reserved and free