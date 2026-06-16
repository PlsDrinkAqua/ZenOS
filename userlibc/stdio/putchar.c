#include <stdio.h>
#include <zenos/syscall.h>

int putchar(int c) {
    zenos_syscall1(SYS_PUTCHAR, c);
    return c;
}
