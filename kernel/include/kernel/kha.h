#ifndef _KHA
#define _KHA

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

void vmm_heap_init(void);

void *vmm_alloc_pages(size_t npages, uint32_t flags);

void vmm_free_pages(void *ptr, size_t npages);

void vmm_heap_test(void);

#endif