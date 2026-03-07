#include <stdint.h>

void test_user_main(void) {
    volatile uint32_t x = 0x12345678;
    (void)x;

    // 打印不了也没关系，先故意执行特权指令
    // 如果已经在 ring3，这里应触发 #GP
    __asm__ volatile ("1: jmp 1b");
    __asm__ volatile ("cli");

    // 不应该执行到这里
    while (1) {
        __asm__ volatile ("jmp .");
    }
}