#ifndef _LIBK_STRING_H
#define _LIBK_STRING_H 1

#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int kmemcmp(const void*, const void*, size_t);
void* kmemcpy(void* __restrict, const void* __restrict, size_t);
void* kmemmove(void*, const void*, size_t);
void* kmemset(void*, int, size_t);
size_t kstrlen(const char*);
int kstrcmp(const char *s1, const char *s2);
char *kstrtok_r(char *str, const char *delim, char **saveptr);
char *kstrcpy(char *dest, const char *src);

#ifdef __cplusplus
}
#endif

#endif
