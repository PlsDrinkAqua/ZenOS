.global jump_usermode
.extern test_user_main

jump_usermode:
    cli
    # 先把用户态数据段装进 ds/es/fs/gs
    movw $0x23, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    # 构造 iret 栈帧，切到 ring3
    movl %esp, %eax
    pushl $0x23              # SS = user data selector
    pushl %eax               # ESP = current esp (demo 用)
    pushfl
    pushl $0x1B              # CS = user code selector
    pushl $test_user_main
    1: jmp 1b
    iret