#include <stdint.h>
#include <kernel/tty.h>
#include <kernel/keyboard.h>

enum {
    SYS_PUTCHAR = 0,
    SYS_WRITE   = 1,
    SYS_GETKEY  = 2,
    SYS_READ    = 3,
    SYS_CLEAR   = 4,
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
            regs->eax = regs->ebx;
            break;

        case SYS_WRITE: {
            if (regs->ebx != 1 && regs->ebx != 2) {
                regs->eax = (uint32_t)-1;
                break;
            }

            const char *buf = (const char *)regs->ecx;
            uint32_t count = regs->edx;

            for (uint32_t i = 0; i < count; i++) {
                terminal_putchar(buf[i]);
            }

            regs->eax = count;
            break;
        }

        case SYS_GETKEY:
            regs->eax = (uint32_t)keyboard_getchar();
            break;

        case SYS_READ: {
            if (regs->ebx != 0) {
                regs->eax = (uint32_t)-1;
                break;
            }

            char *buf = (char *)regs->ecx;
            uint32_t count = regs->edx;
            uint32_t read = 0;

            while (read < count) {
                int c = keyboard_getchar();
                if (c < 0) {
                    break;
                }

                buf[read++] = (char)c;
            }

            regs->eax = read;
            break;
        }

        case SYS_CLEAR:
            terminal_clear();
            regs->eax = 0;
            break;

        default:
            regs->eax = (uint32_t)-1;
            break;
    }
}
