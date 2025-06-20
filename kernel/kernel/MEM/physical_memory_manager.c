#include "kernel/multiboot.h"
#include "kernel/pmm.h"
#include <stdint.h>
#include <stdio.h>

#define ADDR_OFFSET 0xC0000000U



// 返回 mmap entry 的起始物理地址（base）
static inline uint64_t mmap_entry_base(const multiboot_memory_map_t *mmmt) {
    return ((uint64_t)mmmt->addr_high << 32) | mmmt->addr_low;
}

// 返回 mmap entry 的长度
static inline uint64_t mmap_entry_length(const multiboot_memory_map_t *mmmt) {
    return ((uint64_t)mmmt->len_high  << 32) | mmmt->len_low;
}


void mm_init(multiboot_info_t* mbd, uint32_t magic)
{
    /* Make sure the magic number matches for memory mapping*/
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("invalid magic number! \n");
    }else{
        printf("done \n");
    }

    // Convert the physical pointer to its high-half virtual address:
    mbd = (multiboot_info_t*)((uintptr_t)mbd + ADDR_OFFSET);

    

    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        printf("invalid memory map given by GRUB bootloader \n");
    }

    /* Loop through the memory map and display the values */
    printf("Looping through memory map\n");
    int i;
    for(i = 0; i < mbd->mmap_length; 
        i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        mmmt = (multiboot_memory_map_t*)((uintptr_t)mmmt + ADDR_OFFSET);

        /*Because we are in 32 bit system, no need to get info of addr_high and len_high*/
        printf("Start Addr: 0x%x |Length: %x | Size: %x | Type: %d\n",
            mmmt->addr_low,mmmt->len_low , mmmt->size, mmmt->type);

        if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            /* 
             * Do something with this memory block!
             * BE WARNED that some of memory shown as availiable is actually 
             * actively being used by the kernel! You'll need to take that
             * into account before writing to memory!
             */
        }
    }
    printf("done \n");
}

