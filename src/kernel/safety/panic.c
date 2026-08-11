#include "arch_cpu.h"
#include "drivers/uart/uart.h"
#include "kernel/safety/panic.h"
#include "include/board.h"
#include "panic.h"

extern u8 _kernel_start;

const char *panic_messages[] = {
    "The experts call this not good and I call this not good.",
    "Who burnt the toast?",
    "<Insert funny joke>",
    "Call it a feature and ship it!",
    "I blame the t2 register again.",
    "Stack overflow?",
    "Ask r/osdev",
    "At least it didn't pan-Never mind actually.",
    "EVERYBODY PANIC",
    "Wait, why are we panicking?",
    "Time for a devlog entry!",
    "Clearly at least one of the lines of code was wrong.",
    "Rust would probably have fixed this.",
    "Must have been a hardware issue.",
    "For a full description of error load linux and 'cat /dev/urandom' and have a hunt.",
    "Fix for this issue can be found at 'cat /dev/urandom' (on linux computer). Just remove the boilerplate around it!",
    "Must have been a fault with qemu.",
    "Maybe the integer wasn't signed?",
    "Panic has been handled gracefully by forcefully killing everything." ,
    "Was definitely userspace couldn't have been the kernel.",
    "The computer might be tired and needs more sleep.",
    "Might need to download more ram.",
    "ERROR: Couldn't pick a message in time.",
    "ERROR: Message too long.",
    "Error running linu-I mean toastium.",
    "The system as run into a err-INTERUPT: Invalid instruction.-or and wasn't able to recover.",
    "Could have been too many recursion layers deep.",
    "Good luck figuring out what the random data below means lol.",
    "Maybe https://osdev.org talks about this.",
    "Could have trapped badly.",
    "Likely users fault.",
    "Time to commit to a new commit." ,
    "Bad pointer.",
    "Null pointer reference.",
    "Something went wrong, not sure what though.",
    "Might be bed time?",
    "Maybe it was named dinnerbone.",
    "Segmentation Fault Error. Not teapot",
    "Likely missing a semicolon",
    "FATAL : Off by one error... Or maybe off by a few... In a few different places",
    "ERROR: Undefined behavior, Expected to not panic",
    "Ran out of memory trying to remember everything being remembered.",
    "Race condition. System panicked after it lost the bet it had placed",
    "This message has just been printed now! So the code printing this is the latest to run as its being printed.",
    "Interrupt handler error. Was unable to handle it.",
    "Loaded in 65bit mode on mistake.",
    "Kernel lost he pointer, failed finding it.",
    "Throw some more bitwise math at it!",
    "CPU is taking a break from functioning correctly.",
    "Must be a CPU bug.",
    "The RNG was to random.",
    "Im hungry.",
    "What do you think the issue was?",
    "Im not sure...",
    "Not enough operating for the system.",
    "Accidentally clicked panic button.",
    "EMERGENCY MEETING!",
    "There is an imposter among us...",
    "The cake was a lie.",
    "GAME OVER"
};

#define PANIC_MESSAGE_CNT (sizeof(panic_messages) / sizeof(panic_messages[0]))

void kernel_panic(const char *file, const s64 file_line, const char *function, const char *message, s64 extra_value_1, s64 extra_value_2, s64 extra_value_3) {
    uart_println_str("");
    uart_println_str(" _  ________ _____  _   _ ______ _        _____        _   _ _____ _____ ");
    uart_println_str("| |/ /  ____|  __ \\| \\ | |  ____| |      |  __ \\ /\\   | \\ | |_   _/ ____|");
    uart_println_str("| ' /| |__  | |__) |  \\| | |__  | |      | |__) /  \\  |  \\| | | || |     ");
    uart_println_str("|  < |  __| |  _  /| . ` |  __| | |      |  ___/ /\\ \\ | . ` | | || |     ");
    uart_println_str("| . \\| |____| | \\ \\| |\\  | |____| |____  | |  / ____ \\| |\\  |_| || |____ ");
    uart_println_str("|_|\\_\\______|_|  \\_\\_| \\_|______|______| |_| /_/    \\_\\_| \\_|_____\\_____|");

    u64 random_value = time_cnt() + extra_value_1 + extra_value_2 + extra_value_3 + cycle_cnt();

    uart_println_str((char *)panic_messages[random_value % PANIC_MESSAGE_CNT] + (u64)&_kernel_start);
    uart_println_str("");
    uart_println_str("Here is some info relating to what went wrong...");
    uart_println_str("-------------------------------------------------------------------------");

    //file, function and line
    uart_print_str("GIT VERSION HASH : ");
    uart_print_str(COMPILE_VERSION);
    uart_print_str(", COMPILE DATE : ");
    uart_println_str(__DATE__);

    //file, function and line
    uart_print_str("LOCATION : ");
    uart_print_str(file);
    uart_print_str(":");
    uart_print_s64(file_line);
    uart_print_str(" (");
    uart_print_str(function);
    uart_println_str(")");

    //reason
    uart_print_str("REASON   : ");
    uart_println_str(message);

    //prs32 out hex code for num
    uart_print_str("VALUE 1 : 0x");
    uart_print_u64_hex(extra_value_1);
    uart_print_str(" - (");
    uart_print_s64(extra_value_1);
    uart_println_str(")");

    uart_print_str("VALUE 2 : 0x");
    uart_print_u64_hex(extra_value_2);
    uart_print_str(" - (");
    uart_print_s64(extra_value_2);
    uart_println_str(")");

    uart_print_str("VALUE 3 : 0x");
    uart_print_u64_hex(extra_value_3);
    uart_print_str(" - (");
    uart_print_s64(extra_value_3);
    uart_println_str(")");

    freeze_system();
}