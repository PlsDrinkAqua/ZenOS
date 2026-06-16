#ifndef _USERLIBC_STDIO_H
#define _USERLIBC_STDIO_H 1

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int putchar(int);
int getchar(void);
int puts(const char*);

#ifdef __cplusplus
}
#endif

#endif
