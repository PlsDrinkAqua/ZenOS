#ifndef _PMM
#define _PMM

#include "multiboot.h"
#include <stdint.h>

void pmm_init(multiboot_info_t* mbd, uint32_t magic);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t physaddr);
void pmm_test_frame(uint32_t physaddr);


#endif