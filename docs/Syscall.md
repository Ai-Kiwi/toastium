# Syscalls

| Name  | Description | Arg 0 (code) | Arg 1 | Arg 2 | Arg 3 | Response |
|-|-|-|-|-|-|-|
| N/A | Invalid syscall, designed to crash  | 0 | | | | |
| uart_print_char | Prints out a char to uart  | 1 | 0 | Char | | |
| uart_print_str | Prints out a char to uart  | 1 | 1 | Ram Location | String Size | |
| exit | Stops program | 2 | | | | |
| idle_hang | Only should be used by idle process, hangs process but doesn't kill | 3 | | | | |
| open | open path and get file descriptor | 4 | File Path | Flags | | |
| read | uses open file descriptor to read data | 5 | file descriptor | buffer loc | size | |
| write | uses open file descriptor to write data | 6 | file descriptor | buffer loc | size | |
| seek | changes file descriptor seek location | 7 | file descriptor | location | | |

