#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "kernel/vmm.h"       // alloc_frame/free_frame，如果今后需要动态分配页表
#include "kernel/paging.h"
#include "kernel/pmm.h"


#define KERNEL_VIRT_OFFSET 0xC0000000UL
#define PAGE_SIZE       0x1000U                          // 4 KiB

#define VMM_PRESENT  (1<<0)
#define VMM_RW       (1<<1)
#define VMM_USER     (1<<2)


// 从汇编 .bss 区拿到的静态页目录
extern uint32_t boot_page_directory[];

// 当前活跃的页目录
page_directory_t *current_pd;

// 初始化 VMM：把 boot_page_directory 包装成 current_pd，填好 ref_tables
void vmm_init(void) {
    // 1) 指定当前页目录
    current_pd = (page_directory_t*)boot_page_directory;

    // 2) 遍历所有 PDE，挂上已经存在的页表指针，其余清 0
    for (int i = 0; i < PAGE_DIR_ENTRIES; i++) {
        // 如果该 PDE 标记为 present 且是 4 KiB 页表（PS=0）
        if (current_pd->entries[i].present && current_pd->entries[i].page_size == 0) {
            // 取出物理帧号，转成物理地址
            uint32_t phys = current_pd->entries[i].frame << 12;
            // 转成虚拟地址后存到 ref_tables
            current_pd->ref_tables[i] = (page_table_t*)(phys + KERNEL_VIRT_OFFSET);
        } else {
            current_pd->ref_tables[i] = NULL;
        }
    }
}

int vmm_map_page(uintptr_t vaddr, uintptr_t paddr, uint32_t flags) {
    uint32_t pd_idx = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx = (vaddr >> 12) & 0x3FF;

    // 1) 分配/挂载页表
    if (!current_pd->entries[pd_idx].present) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) return -1;
        page_table_t *pt = (page_table_t*)(phys + KERNEL_VIRT_OFFSET);
        memset(pt, 0, sizeof(page_table_t));
        current_pd->entries[pd_idx].frame   = phys >> 12;
        current_pd->entries[pd_idx].present = 1;
        current_pd->entries[pd_idx].rw      = 1;
        current_pd->ref_tables[pd_idx]      = pt;
    }

    // 2) 写 PTE
    page_table_t *pt = current_pd->ref_tables[pd_idx];
    pt->pages[pt_idx].frame   = paddr >> 12;
    pt->pages[pt_idx].present = 1;
    pt->pages[pt_idx].rw      = (flags & VMM_RW)  ? 1 : 0;

    // 3) 刷新 TLB
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");

    return 0;
}

// 返回 0 成功，-1 表示该页本来就没映射
int vmm_unmap_page(uintptr_t vaddr, bool free_frame) {
    uint32_t pd_idx   = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx   = (vaddr >> 12) & 0x3FF;

    // 1) 页目录未映射
    if (!current_pd->entries[pd_idx].present)
        return -1;

    page_table_t *pt = current_pd->ref_tables[pd_idx];
    // 2) 页表项未映射
    if (!pt || !pt->pages[pt_idx].present)
        return -1;

    // 3) 若需要，释放物理页
    if (free_frame) {
        uint32_t phys = pt->pages[pt_idx].frame << 12;
        pmm_free_frame(phys);
    }

    // 4) 清空 PTE
    pt->pages[pt_idx].present = 0;
    pt->pages[pt_idx].frame   = 0;
    pt->pages[pt_idx].rw      = 0;
    pt->pages[pt_idx].user    = 0;

    // 5) 刷新 TLB
    asm volatile("invlpg (%0)" :: "r"(vaddr) : "memory");

    return 0;
}

// 声明 linker script 中的符号
extern uint8_t _kernel_end;

uint32_t vmm_translate(uintptr_t vaddr) {
    uint32_t pd_idx    = (vaddr >> 22) & 0x3FF;
    uint32_t pt_idx    = (vaddr >> 12) & 0x3FF;
    uint32_t page_off  = vaddr & 0xFFF;       // 低 12 位偏移

    // 页目录未映射
    if (!current_pd->entries[pd_idx].present) return 0;

    page_table_t *pt = current_pd->ref_tables[pd_idx];
    // 页表项未映射
    if (!pt->pages[pt_idx].present) return 0;

    // 把 frame 左移 12 位，加上 page_off
    return (pt->pages[pt_idx].frame << 12) | page_off;
}

int vmm_map_region(uintptr_t vstart,
                   uintptr_t vend,
                   uintptr_t pstart,
                   uint32_t flags)
{
    // 向下对齐起点，向上对齐终点
    uintptr_t va = vstart & ~(PAGE_SIZE - 1);
    uintptr_t end = (vend + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    while (va < end) {
        uintptr_t phys;
        if (pstart)
            phys = pstart + (va - vstart);
        else
            phys = pmm_alloc_frame();  // 直接返回物理地址

        if (phys < 0)
            return -1;

        if (vmm_map_page(va, phys, flags) < 0)
            return -1;

        va += PAGE_SIZE;
    }
    return 0;
}

int vmm_unmap_region(uintptr_t vstart,
                     uintptr_t vend,
                     bool free_frames)
{
    uintptr_t va = vstart & ~(PAGE_SIZE - 1);
    uintptr_t end = (vend + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    while (va < end) {
        // 忽略返回值：即使某页没映射也往下走
        vmm_unmap_page(va, free_frames);
        va += PAGE_SIZE;
    }
    return 0;
}


void vmm_test(void) {
    // 1) 申请一个物理页
    uint32_t phys_test = pmm_alloc_frame();
    if (phys_test == -1) {
        printf("vmm_test: pmm_alloc_frame failed\n");
        return;
    }
    printf("Physical page allocated at 0x%x\n", phys_test);

    // 2) 计算测试用的虚拟页——在 kernel_end 后对齐到下一页
    uint32_t kernel_end_va = (uint32_t)&_kernel_end ;
    // 向上取整到最近的 4 KiB 边界
    uint32_t aligned_end = (kernel_end_va + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    // 再下一页
    uint32_t virt_test = aligned_end + PAGE_SIZE;

    printf("Chosen virtual address: 0x%x\n", virt_test);

    // 3) 做映射
    int ret = vmm_map_page(virt_test, phys_test, VMM_PRESENT|VMM_RW);
    if (ret < 0) {
        printf("vmm_map_page failed\n");
        return;
    }
    printf("Mapped VA 0x%x -> PA 0x%x\n", virt_test, phys_test);

    // 4) 验证翻译
    uint32_t trans = vmm_translate(virt_test);
    printf("vmm_translate(0x%x) = 0x%x\n", virt_test, trans);
    if (trans != phys_test) {
        printf("Translation mismatch!\n");
        return;
    }

    // 5) 写读测试
    uint32_t *p = (uint32_t*)virt_test;
    *p = 0xDEADBEEF;
    uint32_t v = *p;
    printf("Wrote 0xDEADBEEF, read back 0x%x -> %s\n",
           v, (v == 0xDEADBEEF) ? "PASS" : "FAIL");
    
    // 取消映射 virt_test，并把物理页一起还给 PMM
    pmm_test_frame(phys_test);
    if (vmm_unmap_page(virt_test, true) == 0) {
        printf("Unmapped VA 0x%x and freed its frame\n", virt_test);
    } else {
        printf("VA 0x%x was not mapped\n", virt_test);
    }

}

// 测试 map_region / unmap_region
void vmm_test_region(void) {
    const int num_pages = 4;

    // 1) 计算起始虚拟地址：kernel_end 向上对齐到页边界，再偏移两页
    uintptr_t kernel_end_va = (uintptr_t)&_kernel_end;
    uintptr_t aligned_end   = (kernel_end_va + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uintptr_t vstart        = aligned_end + 2 * PAGE_SIZE;
    uintptr_t vend          = vstart + num_pages * PAGE_SIZE;

    printf("vmm_test_region: mapping VA 0x%x -> 0x%x (%d pages)\n",
           vstart, vend, num_pages);

    // 2) 批量映射：让每个虚拟页分配一个新的物理页
    if (vmm_map_region(vstart, vend, 0, VMM_PRESENT | VMM_RW) < 0) {
        printf("vmm_map_region failed\n");
        return;
    }
    printf("Region mapped.\n");

    // 3) 验证每页的翻译 & 读写测试
    for (int i = 0; i < num_pages; i++) {
        uintptr_t va = vstart + i * PAGE_SIZE;
        uint32_t pa  = vmm_translate(va);
        printf(" Page %d: VA=0x%x -> PA=0x%x\n", i, va, pa);

        uint32_t *ptr = (uint32_t*)va;
        uint32_t pattern = 0xABCD0000 | i;
        *ptr = pattern;
        uint32_t val = *ptr;
        printf("  Wrote 0x%x, read 0x%x -> %s\n",
               pattern, val, (val == pattern) ? "PASS" : "FAIL");
    }

    // 4) 批量取消映射并释放物理页
    if (vmm_unmap_region(vstart, vend, true) == 0) {
        printf("Region unmapped and frames freed.\n");
    } else {
        printf("vmm_unmap_region failed\n");
        return;
    }

    // 5) 验证取消映射：translate 应都返回 0
    for (int i = 0; i < num_pages; i++) {
        uintptr_t va = vstart + i * PAGE_SIZE;
        uint32_t pa = vmm_translate(va);
        printf(" Page %d: translate(0x%x) = 0x%x -> %s\n",
               i, va, pa, (pa == 0) ? "UNMAPPED" : "STILL MAPPED");
    }
}