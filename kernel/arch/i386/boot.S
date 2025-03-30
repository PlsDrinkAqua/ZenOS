# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16384            # 16 KiB
stack_top:

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
    movl $stack_top, %esp

    # -- Set up our GDT ------------------------------------------
    # Load the address of our GDT descriptor into the CPU.
    lgdt gdt_descriptor
    # Do a far jump to flush the instruction prefetch and load the new CS.
    ljmp $0x08, $flush_gdt
flush_gdt:
    # Reload the data segment registers with the new data segment selector (0x10)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    # -------------------------------------------------------------

    # Call the global constructors.
    call _init

    # Transfer control to the main kernel.
    call kernel_main

    # Hang if kernel_main unexpectedly returns.
    cli
1:  hlt
    jmp 1b
.size _start, . - _start

# --- GDT definitions -----------------------------------------
.section .data
.align 4
gdt_start:
    # Null Descriptor (Descriptor 0)
    .quad 0

    # Code Segment Descriptor (Descriptor 1)
    # Set up a 4 GB segment with base 0.
    .word 0xFFFF           # Limit low: lower 16 bits of limit (0xFFFF)
    .word 0x0000           # Base low: lower 16 bits of base (0x0000)
    .byte 0x00             # Base middle: next 8 bits of base (0x00)
    .byte 0x9A             # Access byte: present, ring 0, code segment, executable & readable (0x9A)
    .byte 0xCF             # Flags & Limit high: 4K granularity, 32-bit op size, high 4 bits of limit (0xCF)
    .byte 0x00             # Base high: last 8 bits of base (0x00)

    # Data Segment Descriptor (Descriptor 2)
    .word 0xFFFF           # Limit low: (0xFFFF)
    .word 0x0000           # Base low: (0x0000)
    .byte 0x00             # Base middle: (0x00)
    .byte 0x92             # Access byte: present, ring 0, data segment, writable (0x92)
    .byte 0xCF             # Flags & Limit high: 4K granularity, 32-bit op size (0xCF)
    .byte 0x00             # Base high: (0x00)

gdt_end:

# The GDT descriptor structure:
#  - First word: size of GDT in bytes minus 1.
#  - Next double word: linear address of the GDT.
gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start





