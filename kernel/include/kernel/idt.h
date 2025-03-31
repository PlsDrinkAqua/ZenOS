#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* An IDT entry structure. Packed to prevent any compiler-added padding. */
struct idt_entry {
    uint16_t base_low;    // Lower 16 bits of the handler address.
    uint16_t sel;         // Kernel segment selector.
    uint8_t always0;      // This must always be 0.
    uint8_t flags;        // Flags (type and attributes).
    uint16_t base_high;   // Upper 16 bits of the handler address.
} __attribute__((packed));

/* IDT pointer structure */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;        // Base address of the IDT.
} __attribute__((packed));

/* Function prototypes */
void idt_install(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif /* IDT_H */
