#include <stdint.h>
#include <kernel/pic.h>
#include <stdio.h>

extern void keyboard_isr();
extern void timer_isr();

typedef struct registers {
    uint32_t edi;         // Last pushed by PUSHAD (lowest memory address)
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;         // Original ESP (pushed by PUSHAD)
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;         // First pushed by PUSHAD (highest memory address)
    uint32_t int_num;      // Manually pushed interrupt number (e.g., 32 for IRQ0)
    uint32_t dummy_error; // Manually pushed dummy error code (e.g., 0)
} registers_t;


void interrupt_handler(registers_t *regs)
{
    switch(regs->int_num) {
        case 32:
            timer_isr();
            break;
        case 33:
            // Call the keyboard interrupt service routine if int_num is 32
            keyboard_isr();
            break;
        case 46:  // IRQ14: Primary ATA
            break;
        case 47:  // IRQ15: Secondary ATA
            break;
        default:
            // For other interrupts, do nothing
            break;
    }
    
    PIC_sendEOI((regs->int_num)-32); //Subtract 32 becuase of the offset
}

void page_fault_handler(uint32_t error_code) {
    uint32_t faulting_address;
    /* 读 CR2 寄存器，获得发生 Page Fault 的线性地址 */
    __asm__ volatile ("mov %%cr2, %0" : "=r"(faulting_address));

    printf("Page Fault! EIP=0x%x, addr=0x%x, err=0x%x\n",
           /* 你可以从栈里再取出返回 EIP，或者简化为： */ 0, 
           faulting_address,
           error_code);

    /* 根据 error_code 位含义，决定是保护性错误还是不存在页。 */
    /* bit 0 = 0 (not-present) / 1 (protection fault) */
    if (!(error_code & 0x1)) {
        printf(" - Page not present\n");
    } else {
        printf(" - Protection violation\n");
    }

    /* … 这里可以做缺页中断处理，或者直接死机 … */
    for (;;);  /* 简单处理：停在这里 */
}