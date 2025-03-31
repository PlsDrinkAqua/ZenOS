#ifndef _GDT_H
#define _GDT_H 1

/* Prototypes for GDT setup functions. */
void gdt_install(void);
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran);
                  
/* gdt_flush() is implemented in assembly. It reloads the new GDT and segment registers. */
extern void gdt_flush(void);

#endif /* SYSTEM_H */
