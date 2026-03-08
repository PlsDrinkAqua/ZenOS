#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "kernel/elf.h"
#include "kernel/ext2_api.h"
#include "kernel/kmalloc.h"
#include "kernel/vmm.h"
#include "kernel/pmm.h"

/* ========== 你需要按自己工程替换/对接的部分开始 ========== */

#define PAGE_SIZE 4096U
#define ALIGN_DOWN(x, a) ((uint32_t)(x) & ~((uint32_t)(a) - 1))
#define ALIGN_UP(x, a)   (((uint32_t)(x) + ((uint32_t)(a) - 1)) & ~((uint32_t)(a) - 1))

/*
 * 下面这几个 hook 你需要改成你自己的函数名。
 *
 * 假设：
 * 1. 你能分配一个物理页 frame，返回物理地址（4KB 对齐）
 * 2. 你能把当前进程页表里 virt -> phys 映射成 user page
 * 3. 内核运行时，当前页表就是目标用户进程页表，因此映射后可以直接 memcpy 到用户虚拟地址
 */

/* 页表 flag，根据你自己的定义改 */
#ifndef VMM_PRESENT
#define VMM_PRESENT  (1u << 0)
#endif
#ifndef VMM_RW
#define VMM_RW       (1u << 1)
#endif
#ifndef VMM_USER
#define VMM_USER     (1u << 2)
#endif

/* ========== 你需要按自己工程替换/对接的部分结束 ========== */

static int elf_check_header(const Elf32_Ehdr *eh, size_t image_size) {
    if (!eh) return -1;
    if (image_size < sizeof(Elf32_Ehdr)) return -1;

    if (eh->e_ident[EI_MAG0] != ELFMAG0 ||
        eh->e_ident[EI_MAG1] != ELFMAG1 ||
        eh->e_ident[EI_MAG2] != ELFMAG2 ||
        eh->e_ident[EI_MAG3] != ELFMAG3) {
        printf("ELF: bad magic\n");
        return -1;
    }

    if (eh->e_ident[EI_CLASS] != ELFCLASS32) {
        printf("ELF: not ELF32\n");
        return -1;
    }

    if (eh->e_ident[EI_DATA] != ELFDATA2LSB) {
        printf("ELF: not little-endian\n");
        return -1;
    }

    if (eh->e_machine != EM_386) {
        printf("ELF: not i386\n");
        return -1;
    }

    if (eh->e_ident[EI_VERSION] != EV_CURRENT || eh->e_version != EV_CURRENT) {
        printf("ELF: bad version\n");
        return -1;
    }

    if (eh->e_type != ET_EXEC) {
        printf("ELF: only ET_EXEC supported now (got %u)\n", eh->e_type);
        return -1;
    }

    if (eh->e_phoff == 0 || eh->e_phnum == 0) {
        printf("ELF: no program headers\n");
        return -1;
    }

    if (eh->e_phentsize != sizeof(Elf32_Phdr)) {
        printf("ELF: unexpected phdr size\n");
        return -1;
    }

    /* 粗略边界检查 */
    uint32_t ph_end = eh->e_phoff + (uint32_t)eh->e_phnum * sizeof(Elf32_Phdr);
    if (ph_end > image_size) {
        printf("ELF: phdr table out of range\n");
        return -1;
    }

    return 0;
}

static int elf_map_segment_pages(uint32_t seg_vaddr, uint32_t seg_memsz, uint32_t seg_flags) {
    uint32_t map_start = ALIGN_DOWN(seg_vaddr, PAGE_SIZE);
    uint32_t map_end   = ALIGN_UP(seg_vaddr + seg_memsz, PAGE_SIZE);

    uint32_t page_flags = VMM_PRESENT | VMM_USER | VMM_RW;

    for (uint32_t va = map_start; va < map_end; va += PAGE_SIZE) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) {
            printf("ELF: pmm_alloc_page failed\n");
            return -1;
        }
        if (vmm_map_page(va, phys, page_flags) < 0) {
            printf("ELF: vmm_map_page failed for va=0x%x\n", va);
            return -1;
        }

        /* 映射完后把整页清零，避免脏数据 */
        memset((void *)va, 0, 1);
    }

    return 0;
}

int elf_load_from_memory(const void *image, size_t image_size, elf_load_result_t *out) {
    if (!image || !out) return -1;

    const uint8_t *file = (const uint8_t *)image;
    const Elf32_Ehdr *eh = (const Elf32_Ehdr *)file;

    if (elf_check_header(eh, image_size) < 0) {
        return -1;
    }

    const Elf32_Phdr *phdrs = (const Elf32_Phdr *)(file + eh->e_phoff);

    uint32_t max_loaded_end = 0;
    

    for (uint32_t i = 0; i < eh->e_phnum; i++) {
        const Elf32_Phdr *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD) {
            continue;
        }

        if (ph->p_memsz == 0) {
            continue;
        }

        /* 基本合法性检查 */
        if (ph->p_filesz > ph->p_memsz) {
            printf("ELF: segment filesz > memsz\n");
            return -1;
        }

        if ((uint64_t)ph->p_offset + ph->p_filesz > image_size) {
            printf("ELF: segment exceeds file image\n");
            return -1;
        }

        /*
         * 先把这个 segment 覆盖的用户虚拟地址范围映射好
         */
        if (elf_map_segment_pages(ph->p_vaddr, ph->p_memsz, ph->p_flags) < 0) {
            return -1;
        }

        /*
         * 拷贝文件中的实际内容到 p_vaddr
         * 注意：这里默认“当前页表”已经是目标用户地址空间，所以 ring0 下可以直接写这个用户虚拟地址
         */
        if (ph->p_filesz > 0) {
            memcpy((void *)ph->p_vaddr, file + ph->p_offset, ph->p_filesz);
        }

        /*
         * .bss 或 memsz > filesz 的部分清零
         * 虽然前面整页清过一次，这里还是把逻辑上段内剩余部分明确清零
         */
        if (ph->p_memsz > ph->p_filesz) {
            memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }

        uint32_t seg_end = ph->p_vaddr + ph->p_memsz;
        if (seg_end > max_loaded_end) {
            max_loaded_end = seg_end;
        }

        printf("ELF: LOAD seg %u vaddr=0x%x filesz=%u memsz=%u flags=0x%x\n",
               i, ph->p_vaddr, ph->p_filesz, ph->p_memsz, ph->p_flags);
    }

    if (max_loaded_end == 0) {
        printf("ELF: no PT_LOAD segment found\n");
        return -1;
    }

    out->entry      = eh->e_entry;
    out->heap_start = ALIGN_UP(max_loaded_end, PAGE_SIZE);
    out->heap_end   = out->heap_start;

    printf("ELF: entry=0x%x heap_start=0x%x\n", out->entry, out->heap_start);
    return 0;
}

int elf_load_from_file(const char *path, elf_load_result_t *out) {
    if (!path || !out) return -1;

    int fd = ext2_open(path);
    if (fd < 0) {
        printf("ELF: ext2_open(\"%s\") failed\n", path);
        return -1;
    }

    size_t size = ext2_filesize(fd);
    if (size == 0) {
        printf("ELF: empty file or ext2_filesize failed\n");
        ext2_close(fd);
        return -1;
    }

    uint8_t *buf = kmalloc(size);
    if (!buf) {
        printf("ELF: kmalloc(%u) failed\n", (unsigned)size);
        ext2_close(fd);
        return -1;
    }

    size_t n = ext2_read(fd, buf, size);
    ext2_close(fd);

    if (n != size) {
        printf("ELF: ext2_read incomplete (%u/%u)\n", (unsigned)n, (unsigned)size);
        kfree(buf);
        return -1;
    }

    int ret = elf_load_from_memory(buf, size, out);
    kfree(buf);
    return ret;
}