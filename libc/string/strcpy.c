#include <string.h>

char *strcpy(char *dest, const char *src) {
    char *p = dest;
    while ((*p++ = *src++) != '\0')
        ;  // body intentionally empty
    return dest;
}