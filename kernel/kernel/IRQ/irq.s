.global  _irq0
.align   4
_irq0:
   cli
   pushl $0           # Push dummy error code (0)
   pushl $32          # Push interrupt number (32)
   jmp irq_common_stub

.global  _irq1
.align   4
_irq1:
   cli
   pushl $0           # Push dummy error code (0)
   pushl $33          # Push interrupt number (32)
   jmp irq_common_stub

    
.global   irq_common_stub
.align   4
irq_common_stub:
    pushal             # Save all general-purpose registers
    cld                # Clear DF for C code

    movl %esp, %eax    # EAX now points to the register block (includes dummy error code and int number)
    pushl %eax      	# Push pointer as argument to the C handler
    
    call    interrupt_handler  # Call your C handler
    addl $4, %esp      # Clean up the argument (pointer)

    popal              # Restore general-purpose registers
    addl $8, %esp      # Clean up the two pushed values: dummy error code and interrupt number
    iret               # Return from interrupt
