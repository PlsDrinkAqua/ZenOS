#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "kernel/kha.h"
#include "kernel/vmm.h"
#include "kernel/kmalloc.h"

#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define KMALLOC_ALIGN    8
#define PAGE_SIZE       0x1000U                          // 4 KiB

#define VMM_PRESENT  (1<<0)
#define VMM_RW       (1<<1)
#define VMM_USER     (1<<2)

typedef struct kmem_block {
    size_t size;               // size of this block *including* header
    struct kmem_block *next;   // next free block
} kmem_block_t;

static kmem_block_t *free_list = NULL;
// static uintptr_t    heap_end  = 0;  // tracks how far we’ve grown our heap

static kmem_block_t* expand_heap(size_t need) {
    // round total request (need + header) up to pages
    size_t total = ALIGN_UP(need + sizeof(kmem_block_t), PAGE_SIZE);
    size_t npages = total / PAGE_SIZE;

    void *v = vmm_alloc_pages(npages, VMM_PRESENT | VMM_RW);
    if (!v) return NULL;

    // create one big free block covering that range
    kmem_block_t *blk = (kmem_block_t*)v;
    blk->size = npages * PAGE_SIZE;
    blk->next = NULL;
    return blk;
}

void *kmalloc(size_t sz) {
    if (sz == 0) return NULL;
    // align
    sz = ALIGN_UP(sz, KMALLOC_ALIGN);

    kmem_block_t **prev = &free_list;
    kmem_block_t  *cur  = free_list;

    // 1) search for a free block large enough
    while (cur) {
        if (cur->size >= sz + sizeof(kmem_block_t)) {
            // found it — detach from free list
            *prev = cur->next;

            // optionally split if the remainder is big enough
            size_t leftover = cur->size - (sz + sizeof(kmem_block_t));
            if (leftover >= sizeof(kmem_block_t) + KMALLOC_ALIGN) {
                kmem_block_t *rem = (kmem_block_t*)((uint8_t*)cur
                                            + sizeof(kmem_block_t) + sz);
                rem->size = leftover;
                rem->next = free_list;
                free_list = rem;

                cur->size = sz + sizeof(kmem_block_t);
            }

            // return address just past the header
            return (void*)(cur + 1);
        }
        prev = &cur->next;
        cur  = cur->next;
    }

    // 2) no block found — grow the heap
    kmem_block_t *blk = expand_heap(sz);
    if (!blk) return NULL;

    // if it’s bigger than we need, split as above:
    size_t extra = blk->size - (sz + sizeof(kmem_block_t));
    if (extra >= sizeof(kmem_block_t) + KMALLOC_ALIGN) {
        kmem_block_t *rem = (kmem_block_t*)((uint8_t*)blk
                                    + sizeof(kmem_block_t) + sz);
        rem->size = extra;
        rem->next = free_list;
        free_list  = rem;

        blk->size = sz + sizeof(kmem_block_t);
    }

    return (void*)(blk + 1);
}


void kfree(void *ptr) {
    if (!ptr) return;
    kmem_block_t *blk = ((kmem_block_t*)ptr) - 1;

    // simple: push to free_list head
    blk->next   = free_list;
    free_list   = blk;

    // TODO: you can walk free_list and merge any two adjacent blocks
    // if (blk + blk->size == blk->next) { /* coalesce */ }
}


// void kmalloc_test(void) {
//     printf("=== kmalloc/kfree test start ===\n");

//     // 1) 简单小块分配
//     void *p1 = kmalloc(24);
//     printf("Allocated p1 (24 bytes): 0x%x\n", p1);
//     if (!p1) {
//         printf("  [ERROR] kmalloc returned NULL\n");
//         return;
//     }
//     // 写入并验证数据完整性
//     memset(p1, 0xAA, 24);
//     for (size_t i = 0; i < 24; i++) {
//         if (((uint8_t*)p1)[i] != 0xAA) {
//             printf("  [ERROR] data corruption at p1[%d]\n", (unsigned)i);
//             break;
//         }
//     }

//     // 2) 再申请一个中等大小块
//     void *p2 = kmalloc(40);
//     printf("Allocated p2 (40 bytes): 0x%x\n", p2);

//     // 3) 释放 p1，测试下次分配能否重用
//     kfree(p1);
//     printf("Freed p1\n");

//     void *p3 = kmalloc(16);
//     printf("Allocated p3 (16 bytes): 0x%x  (expected == p1)\n", p3);

//     // 4) 释放剩余
//     kfree(p2);
//     printf("Freed p2\n");
//     kfree(p3);
//     printf("Freed p3\n");

//     // 5) 大块分配：测试堆扩展逻辑
//     size_t big_size = PAGE_SIZE * 2 + 100;
//     void *p4 = kmalloc(big_size);
//     printf("Allocated p4 (%u bytes): 0x%x\n", big_size, p4);
//     if (!p4) {
//         printf("  [ERROR] large kmalloc returned NULL\n");
//     } else {
//         // 可选：向首尾写入，保证这块新页可用
//         memset(p4, 0x55, 1);
//         memset((uint8_t*)p4 + big_size - 1, 0x55, 1);
//     }
//     kfree(p4);
//     printf("Freed p4\n");

//     printf("=== kmalloc/kfree test complete ===\n");
// }
