#include <unistd.h>
#include <zenos/syscall.h>

int exec(const char *path) {
    return zenos_syscall1(SYS_EXEC, (int)path);
}
