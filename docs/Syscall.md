# Syscalls

| Name  | Description | Arg 0 (code) | Arg 1 | Arg 2 | Arg 3 | Response |
|-|-|-|-|-|-|-|
| N/A | Invalid syscall, designed to crash  | 0 | | | | |
| uart_print_char | Prints out a char to uart  | 1 | 0 | Char | | |
| uart_print_str | Prints out a char to uart  | 1 | 1 | Ram Location | String Size | |
| exit | Stops program | 2 | | | | |


