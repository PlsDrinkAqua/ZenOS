#include <string.h>

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    // 转为 unsigned char 确保差值正确
    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}