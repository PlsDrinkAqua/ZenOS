void _start(void) {
    __asm__ volatile (
        "int $0x80"
        : 
        : "a"(0), "b"('N')
        : "memory"
    );
    __asm__ volatile (
        "int $0x80"
        : 
        : "a"(0), "b"('I')
        : "memory"
    );
    __asm__ volatile (
        "int $0x80"
        : 
        : "a"(0), "b"('G')
        : "memory"
    );
    __asm__ volatile (
        "int $0x80"
        : 
        : "a"(0), "b"('G')
        : "memory"
    );
    __asm__ volatile (
        "int $0x80"
        : 
        : "a"(0), "b"('A')
        : "memory"
    );
    for (;;);
}