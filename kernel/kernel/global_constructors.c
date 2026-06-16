#include <libk/stdio.h>
#include <kernel/tty.h>
__attribute__((constructor)) 
void foo(void){
    // terminal_initialize();
    // kprintf("Terminal Initilized\n");
}