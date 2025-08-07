#include <string.h>
#include <stdint.h>

char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *start;
    char *p;

    if (str) {
        start = str;
    } else if (*saveptr) {
        start = *saveptr;
    } else {
        return NULL;
    }

    /* 跳过开头所有的分隔符 */
    p = start;
    while (*p) {
        const char *d = delim;
        int is_delim = 0;
        while (*d) {
            if (*p == *d) { is_delim = 1; break; }
            d++;
        }
        if (!is_delim) break;
        p++;
    }
    if (*p == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    /* 找到 token 开头 */
    start = p;

    /* 找到 token 末尾（遇到第一个分隔符） */
    while (*p) {
        const char *d = delim;
        int is_delim = 0;
        while (*d) {
            if (*p == *d) { is_delim = 1; break; }
            d++;
        }
        if (is_delim) break;
        p++;
    }

    /* 如果没到字符串末尾，替换分隔符为 '\0' 并更新 saveptr */
    if (*p) {
        *p = '\0';
        *saveptr = p + 1;
    } else {
        *saveptr = NULL;
    }

    return start;
}