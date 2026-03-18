.global isr128
.align 4
isr128:
    # 注意：这里是从 ring3 进来的
    # CPU 已经自动压了 user 的 ss/esp/eflags/cs/eip 到内核栈

    pushl $0              # dummy error code
    pushl $128            # int number

    pusha

    # 如果你在 C 里会访问内核数据，最好切段寄存器到 kernel selector
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl %esp            # 传 registers_t* 给 C
    call syscall_handler
    addl $4, %esp

    popa
    addl $8, %esp         # 弹掉 int_num + dummy_error
    iret