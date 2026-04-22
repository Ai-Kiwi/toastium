#include "include/types.h"

bool8 str_starts_with(const char *str, const char *prefix) {
    while (*prefix != '\0') {
        if (*prefix != *str) {
            return FALSE;
        }
        str++;
        prefix++;
    }
    return TRUE;
}
