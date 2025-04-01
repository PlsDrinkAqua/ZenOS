#include "kernel/idt.h"
#include <string.h>  // For memset, if available in your freestanding libc

/* Define an IDT with 256 entries. */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* Sets an individual IDT entry */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].base_low  = base & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
    idt[num].base_high = (base >> 16) & 0xFFFF;
}

/* Inline assembly function to load the IDT */
static inline void idt_flush(void)
{
    __asm__ volatile (
        "lidt (%0)"
        :
        : "r" (&idtp)
        : "memory"
    );
}
extern void _irq0(void);
extern void _irq1(void);

/* Install the IDT by setting up the pointer and defining entries. */
void idt_install(void)
{
    /* Set up the IDT pointer */
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (uint32_t)&idt;

    /* Clear the IDT */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);


    /* Example: Set gate for interrupt 0 (divide-by-zero exception) 
       Note: You must implement the ISR (interrupt service routine) for each interrupt.
       For example, suppose you have an ISR function "isr0" defined in assembly:
         extern void isr0();
       Then set the gate like:
         idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
       Here 0x08 is your code segment selector, and 0x8E means present, ring 0, 32-bit interrupt gate.
    */
    // idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);

    /* Set additional IDT entries as needed here... */
    
    /* Set the keyboard ISR (IRQ1 remapped to vector 0x21) */
    idt_set_gate(0x20, (uint32_t)_irq0, 0x08, 0x8E);
    idt_set_gate(0x21, (uint32_t)_irq1, 0x08, 0x8E);

    /* Load the new IDT */
    idt_flush();
}
