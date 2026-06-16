#include <libk/stdio.h>

int kputs(const char* string) {
	return kprintf("%s\n", string);
}
