#ifndef _USERLIBC_UNISTD_H
#define _USERLIBC_UNISTD_H 1

#include <stddef.h>

typedef int ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

ssize_t write(int fd, const void *buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif
