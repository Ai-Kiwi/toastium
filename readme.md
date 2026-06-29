# Toastium
(Stands for Toast-Is-Utterly-aMazing)

An operating system that I have been developing for learning purposes. Designed for targeting a custom portable console I am working on with someone in the long run, However is designed with portablity in mind so can be ported to other devices or architectures.

## Current Features
 - Preemptive multitasking
 - Basic round robin scheduler with running processes.
 - Syscalls (interruptable and non-interuptable versions) (also handles reading userspace with page fault handling)
 - Running processes
 - Bootloader (U-Boot + OpenSBI for RISC-V)
 - Pager
 - Allocator (Slab allocator)
 - DTB parser
 - Interrupt/exception handler (Internally referred to as Traps)
 - Virtual memory

If you are interested in reading about this project, the devlogs can be found at: https://github.com/Ai-Kiwi/ai-kiwi-devlog/tree/main/toastium
