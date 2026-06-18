#include <libk/stdio.h>
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
#include <kernel/user_heap.h>
#include <kernel/ata.h>
#include <kernel/ext2.h>
#include <kernel/ext2_api.h>
#include <kernel/elf.h>
#include <kernel/tss.h>

#define USER_STACK_TOP 0xBFFFE000
#define VMM_PRESENT  (1<<0)
#define VMM_RW       (1<<1)
#define VMM_USER     (1<<2)
extern void jump_usermode(uint32_t entry, uint32_t user_stack_top);
extern uint8_t stack_top;

int debug = 4;

void kernel_main(multiboot_info_t* mbd, uint32_t magic) {
    terminal_initialize();
    kprintf("Terminal Initilized\n");

    kprintf("Initilizing GDT.................");
	gdt_install();
	kprintf("done \n");

	kprintf("Initilizing IDT.................");
	idt_install();
	kprintf("done \n");

	kprintf("Initilizing Global Constructor.................");
	__asm__ volatile ("call _init");
	kprintf("done \n");

	kprintf("Initilizing Physical Memory Manager.................");
	pmm_init(mbd , magic);
	kprintf("done \n");

	kprintf("Initilizing Virtual Memory Manager.................");
	vmm_init();
	kprintf("done \n");

	kprintf("Initilizing Kernel Heap Allocator.................");
	vmm_heap_init();
	kprintf("done \n");

	kprintf("Initilizing PIC.................");
	PIC_remap(32, 40);
	// 屏蔽 ATA 主从中断（14 和 15）
    IRQ_set_mask(14);   // 屏蔽主 ATA（原 IRQ14）
    IRQ_set_mask(15);   // 屏蔽从 ATA（原 IRQ15）
	kprintf("done \n");

	kprintf("Initilizing EXT2 File System.................");
	ext2_init();
	kprintf("done \n");

	kprintf("Initilizing User Heap Allocator.................");
	user_heap_init();
	kprintf("done \n");

        kprintf("Loading User Programme.................");
        elf_load_result_t res;
        int ret = elf_load_from_file("/shell", &res);
        kprintf("elf_load ret=%d entry=0x%x heap=0x%x\n", ret, res.entry, res.heap_start);
        if (ret < 0) {
            kprintf("failed to load /shell\n");
            for (;;) {
                __asm__ volatile ("hlt");
            }
        }
        kprintf("\n");
        kprintf("done \n");

	kprintf("Creating User Stack.................");
	// 先给 user stack 映射一页
	uintptr_t stack_page = USER_STACK_TOP - 0x1000;
	uint32_t stack_phys = pmm_alloc_frame();
	vmm_map_page(stack_page, stack_phys, VMM_PRESENT | VMM_RW | VMM_USER);
	kmemset((void *)stack_page, 0, 0x1000);
	kprintf("done \n");


	// Enable interrupts globally.
	kprintf("Enabling interrupts.................");
	__asm__ volatile ("sti");
	// asm volatile ("1: jmp 1b");
	kprintf("done \n");
    kprintf("About to enter user mode...\n");
	kprintf("entry: 0x%x, stack: 0x%x",res.entry,USER_STACK_TOP);

	uint32_t kernel_stack_top = (uint32_t)&stack_top;
	tss_set_kernel_stack(kernel_stack_top);

	jump_usermode(res.entry, USER_STACK_TOP);

    kprintf("ERROR: returned from user mode switch!\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}
