#include "include/types.h"

bool8 str_starts_with(const u8 *str, const u8 *prefix) {
    while (*prefix != '\0') {
        if (*prefix != *str) {
            return FALSE;
        }
        str++;
        prefix++;
    }
    return TRUE;
}
