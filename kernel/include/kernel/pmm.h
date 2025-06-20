#include "multiboot.h"
#include <stdint.h>

#ifndef _PMM
#define _PMM
void mm_init(multiboot_info_t* mbd, uint32_t magic);

#endif