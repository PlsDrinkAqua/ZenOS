#ifndef _VMM
#define _VMM

#include <stdint.h>
#include <stdbool.h>

void vmm_init();

int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags);

uint32_t vmm_translate(uintptr_t vaddr);

int vmm_unmap_page(uintptr_t vaddr, bool free_frame);

int vmm_map_region(uintptr_t vstart,
                   uintptr_t vend,
                   uintptr_t pstart,
                   uint32_t flags);

int vmm_unmap_region(uintptr_t vstart,
                     uintptr_t vend,
                     bool free_frames);

void vmm_test();

void vmm_test_region(void);

#endif