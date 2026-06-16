#ifndef _LIBK_STDIO_H
#define _LIBK_STDIO_H 1

#include <sys/cdefs.h>

#define KEOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int kprintf(const char* __restrict, ...);
int kputchar(int);
int kputs(const char*);

#ifdef __cplusplus
}
#endif

#endif
