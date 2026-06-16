#include <libk/stdio.h>
#include <libk/stdlib.h>

__attribute__((__noreturn__))
void kabort(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	kprintf("kernel: panic: kabort()\n");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	kprintf("kabort()\n");
#endif
	while (1) { }
	__builtin_unreachable();
}
