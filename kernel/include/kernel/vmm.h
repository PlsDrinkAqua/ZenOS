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

// void vmm_test();

void vmm_test_region(void);

typedef struct {
    bool     present;   // PTE.P present?
    bool     writable;  // PTE.RW writable?
    bool     user;      // PTE.U user-accessible?
    uintptr_t paddr;    // 物理页基址（仅当 present 时有效）
} vmm_page_info_t;

void dump_boot_pte(uint32_t va);


#endif