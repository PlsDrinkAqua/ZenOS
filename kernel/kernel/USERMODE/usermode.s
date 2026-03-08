.global jump_usermode
jump_usermode:
    cli

    # 先把用户态数据段装进 ds/es/fs/gs
    movw $0x23, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    # 取函数参数
    movl 4(%esp), %ecx       # entry
    movl 8(%esp), %eax       # user esp

    # 构造 iret 栈帧，切到 ring3
    pushl $0x23              # SS = user data selector
    pushl %eax               # ESP = user esp
    pushfl
    pushl $0x1B              # CS = user code selector
    pushl %ecx               # EIP = entry
    iret