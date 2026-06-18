#include <unistd.h>
#include <zenos/syscall.h>

ssize_t read(int fd, void *buf, size_t count) {
    return zenos_syscall3(SYS_READ, fd, (int)buf, (int)count);
}
