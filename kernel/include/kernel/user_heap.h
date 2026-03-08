#ifndef USER_HEAP_H
#define USER_HEAP_H

#include <stdint.h>

void user_heap_init(void);
void *sys_sbrk(intptr_t increment);

#endif