#include <zenos/syscall.h>

int zenos_syscall1(int num, int arg1) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1)
        : "memory"
    );

    return ret;
}

int zenos_syscall3(int num, int arg1, int arg2, int arg3) {
    int ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory"
    );

    return ret;
}
