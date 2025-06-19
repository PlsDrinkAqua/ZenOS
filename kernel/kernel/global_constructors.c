#include <stdio.h>
#include <kernel/tty.h>
__attribute__((constructor)) 
void foo(void){
    // terminal_initialize();
    // printf("Terminal Initilized\n");
}