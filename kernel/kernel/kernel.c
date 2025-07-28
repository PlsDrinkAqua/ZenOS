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
#include <kernel/ext2.h>

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

	printf("Initilizing Virtual Memory Manager.................\n");
	vmm_init();
	printf("done \n");

	printf("Initilizing Kernel Heap Allocator.................");
	vmm_heap_init();
	printf("done \n");

	printf("Initilizing PIC.................");
	PIC_remap(32, 40);
	// 屏蔽 ATA 主从中断（14 和 15）
    IRQ_set_mask(14);   // 屏蔽主 ATA（原 IRQ14）
    IRQ_set_mask(15);   // 屏蔽从 ATA（原 IRQ15）
	printf("done \n");


	printf("Initilizing ATA PIO Driver.................");
	block_devices_init();
	printf("done \n");

	printf("Initilizing EXT2 File System.................");
	ext2_init();
	printf("done \n");


	// Enable interrupts globally.
	printf("Enabling interrupts.................");
	__asm__ volatile ("sti");
	// asm volatile ("1: jmp 1b");
	printf("done \n");
	printf("Hello, kernel World!\n");
	while(1){

	}
}