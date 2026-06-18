#include <stdlib.h>
#include <zenos/syscall.h>

void exit(int status) {
    zenos_syscall1(SYS_EXIT, status);

    for (;;) {
    }
}
