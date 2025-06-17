#include <stdio.h>
#include <kernel/tty.h>
__attribute__((constructor)) 
void foo(void){
    // terminal_initialize();
    //printf("foo is running and printf is available at this point\n");
}