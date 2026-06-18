#ifndef _USERLIBC_STRING_H
#define _USERLIBC_STRING_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char*);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);

#ifdef __cplusplus
}
#endif

#endif
