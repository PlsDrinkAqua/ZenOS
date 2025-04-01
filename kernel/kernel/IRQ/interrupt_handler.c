#include <stdint.h>
#include <kernel/pic.h>

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
        default:
            // For other interrupts, do nothing
            break;
    }
    
    PIC_sendEOI((regs->int_num)-32); //Subtract 32 becuase of the offset
}