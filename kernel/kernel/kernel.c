#include <stdio.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pic.h>
#include <kernel/irq.h>

int debug = 4;

void kernel_main(void) {
	terminal_initialize();
	PIC_remap(32, 40);
	// Enable the IRQ for the keyboard by clearing its mask.
	// IRQ_clear_mask(0);

	// Enable interrupts globally.
	__asm__ volatile ("sti");
	printf("Hello, kernel World!\n");
	while(1){

	}
}
