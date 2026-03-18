#include <stdint.h>
#include <kernel/tty.h>

enum {
    SYS_PUTCHAR = 0,
    SYS_WRITE   = 1,
    SYS_GETKEY  = 2,
};

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


void syscall_handler(registers_t *regs)
{
    switch (regs->eax) {
        case SYS_PUTCHAR:
            terminal_putchar(regs->ebx);
            break;

        default:
            regs->eax = (uint32_t)-1;
            break;
    }
}