#include <sys/gdt.h>
#define NUM_ENTRIES 3

/* A GDT entry is 8 bytes long. We pack the structure to prevent any unwanted padding. */
struct gdt_entry {
    unsigned short limit_low;      /* Lower 16 bits of limit */
    unsigned short base_low;       /* Lower 16 bits of base address */
    unsigned char  base_middle;    /* Next 8 bits of base address */
    unsigned char  access;         /* Access flags, determine what ring this segment can be used in */
    unsigned char  granularity;    /* Granularity, and upper 4 bits of limit */
    unsigned char  base_high;      /* Last 8 bits of base address */
} __attribute__((packed));

/* A GDT pointer is 6 bytes: two for the limit, and four for the base address. */
struct gdt_ptr {
    unsigned short limit;          /* Size of the GDT in bytes minus 1 */
    unsigned int   base;           /* Linear address of the GDT */
} __attribute__((packed));

/* Our GDT with 3 entries. */
struct gdt_entry gdt[NUM_ENTRIES];
struct gdt_ptr gp;

/* Setup a descriptor in the GDT.
 * num: index into the GDT.
 * base: start address of the segment.
 * limit: size of the segment.
 * access: access flags.
 * gran: granularity and other flags.
 */
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

/* gdt_install() sets up the special GDT pointer, creates our 3 GDT entries,
 * and then calls gdt_flush() to load the new GDT and update the segment registers.
 */
void gdt_install(void)
{
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * NUM_ENTRIES) - 1;
    gp.base  = (unsigned int)&gdt;

    /* Our NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Code Segment Descriptor:
     * Base = 0, Limit = 4GB, Access = 0x9A (code segment, executable, readable, accessed),
     * Granularity = 0xCF (4K granularity, 32-bit op size).
     */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Data Segment Descriptor:
     * Base = 0, Limit = 4GB, Access = 0x92 (data segment, writable, accessed),
     * Granularity = 0xCF.
     */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes */
    gdt_flush();
}

void gdt_flush(void)
{
    __asm__ volatile (
        "lgdt (%0)              \n\t"  /* Load our GDT pointer from gp */
        "movw $0x10, %%ax       \n\t"  /* Data segment selector (index 2: 0x10) */
        "movw %%ax, %%ds        \n\t"
        "movw %%ax, %%es        \n\t"
        "movw %%ax, %%fs        \n\t"
        "movw %%ax, %%gs        \n\t"
        "movw %%ax, %%ss        \n\t"
        "ljmp $0x08, $1f   \n\t"  /* Far jump to reload CS (index 1: 0x08) */
        "1:              \n\t"
        :
        : "r" (&gp)
        : "ax", "memory"
    );
}