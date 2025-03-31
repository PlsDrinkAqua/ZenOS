#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/pic.h>

void kernel_main(void) {
	// terminal_initialize();
	PIC_remap(32, 40);
	printf("Hello, kernel World!\n");
}
