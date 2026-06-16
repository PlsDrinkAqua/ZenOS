#include <stdio.h>
#include <zenos/syscall.h>

int getchar(void) {
    return zenos_syscall1(SYS_GETKEY, 0);
}
