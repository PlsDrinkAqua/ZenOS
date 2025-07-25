#include <stdio.h>
#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/pic.h>
#include <kernel/irq.h>
#include <kernel/multiboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/kha.h>
#include <kernel/kmalloc.h>
#include <kernel/ata.h>

int debug = 4;

void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
    terminal_initialize();
    printf("Terminal Initilized\n");

    printf("Initilizing GDT.................");
	gdt_install();
	printf("done \n");

	printf("Initilizing IDT.................");
	idt_install();
	printf("done \n");

	printf("Initilizing Global Constructor.................");
	__asm__ volatile ("call _init");
	printf("done \n");

	printf("Initilizing Physical Memory Manager.................");
	pmm_init(mbd , magic);
	printf("done \n");

	printf("Initilizing Virtual Memory Manager.................");
	vmm_init();
	printf("done \n");

	printf("Initilizing Kernel Heap Allocator.................");
	vmm_heap_init();
	printf("done \n");

	printf("Initilizing PIC.................");
	PIC_remap(32, 40);
	printf("done \n");

    kmalloc_test();
	disk_test();
	// Enable the IRQ for the keyboard by clearing its mask.
	// IRQ_clear_mask(0);

	// Enable interrupts globally.
	printf("Enabling interrupts.................");
	__asm__ volatile ("sti");
	// asm volatile ("1: jmp 1b");
	printf("done \n");
	printf("Hello, kernel World!\n");
	while(1){

	}
}