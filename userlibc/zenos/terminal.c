#include <zenos/syscall.h>
#include <zenos/terminal.h>

int zenos_terminal_clear(void) {
    return zenos_syscall1(SYS_CLEAR, 0);
}
