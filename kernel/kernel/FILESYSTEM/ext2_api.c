#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel/ext2_api.h"
#include "kernel/ext2.h"
#include "kernel/ata.h"
#include "kernel/kmalloc.h"
#include "kernel/vmm.h"

#define MAX_FD 16
#define EXT2_ROOT_INO 2    /* ext2 根目录的 inode 编号 */

static ext2_file_t file_table[MAX_FD];

int ext2_init(void) {
    block_devices_init();;
    if (ext2_driver_init() < 0){
        printf("EXT2 initilization failed!\n");
        return -1;
    }
    memset(file_table, 0, sizeof(file_table));
    return 0;
}

int ext2_read_dir(uint32_t dir_ino,
                  void (*entry_cb)(const char *name, uint32_t ino))
{
    // 1) 读取目录 inode
    struct ext2_inode dir_inode;
    if (ext2_read_inode(dir_ino, &dir_inode) < 0)
        return -1;

    // 2) 计算块大小，申请缓冲
    const struct ext2_super_block *sb = ext2_sb();
    uint32_t block_size = 1024U << sb->s_log_block_size;
    uint8_t *buf = kmalloc(block_size);
    if (!buf) return -1;

    // 3) 遍历直接块
    for (int i = 0; i < 12; i++) {
        uint32_t blk = dir_inode.i_block[i];
        if (!blk) continue;
        if (ext2_read_block(blk, buf) < 0) continue;

        // 4) 在块中解析目录项
        uint32_t offset = 0;
        while (offset < block_size) {
            struct ext2_dir_entry_2 *de = (void*)(buf + offset);
            if (de->inode) {
                char name[256];
                memcpy(name, de->name, de->name_len);
                name[de->name_len] = '\0';
                entry_cb(name, de->inode);
            }
            if (de->rec_len < 8) break;
            offset += de->rec_len;
        }
    }

    // 5) 清理并退出
    kfree(buf);
    return 0;
}


/**
 * 在目录 dir_ino 中查名字 name，返回对应的 inode（找不到返回 0）
 */
uint32_t ext2_lookup(uint32_t dir_ino, const char *name) {
    struct ext2_inode dir_inode;
    if (ext2_read_inode(dir_ino, &dir_inode) < 0)
        return 0;

    const struct ext2_super_block *sb = ext2_sb();
    uint32_t block_size = 1024U << sb->s_log_block_size;
    uint8_t *buf = kmalloc(block_size);
    if (!buf) return 0;

    for (int i = 0; i < 12; i++) {
        uint32_t blk = dir_inode.i_block[i];
        if (!blk) continue;
        if (ext2_read_block(blk, buf) < 0) continue;

        uint32_t offset = 0;
        while (offset < block_size) {
            struct ext2_dir_entry_2 *de = (void*)(buf + offset);
            if (de->inode) {
                /* 取出名字，注意 name_len 不是 '\0' 结尾 */
                char entry_name[256];
                uint32_t len = de->name_len;
                if (len >= sizeof(entry_name)) len = sizeof(entry_name) - 1;
                memcpy(entry_name, de->name, len);
                entry_name[len] = '\0';

                if (strcmp(entry_name, name) == 0) {
                    uint32_t found = de->inode;
                    kfree(buf);
                    return found;
                }
            }
            if (de->rec_len < 8) break;
            offset += de->rec_len;
        }
    }

    kfree(buf);
    return 0;
}


/**
 * 打开一个绝对路径（只支持以 '/' 开头），
 * 返回 fd (0..MAX_FD-1)，失败返回 -1
 */
int ext2_open(const char *path) {
    if (!path || path[0] != '/')
        return -1;

    /* 复制一份可改写的路径 */
    char tmp[strlen(path) + 1];
    strcpy(tmp, path);

    /* 从根 inode 开始逐级 lookup */
    uint32_t ino = EXT2_ROOT_INO;
    char *saveptr, *tok = strtok_r(tmp, "/", &saveptr);
    while (tok) {
        ino = ext2_lookup(ino, tok);
        if (ino == 0)
            return -1;
        tok = strtok_r(NULL, "/", &saveptr);
    }

    /* 分配一个 fd 槽 */
    for (int fd = 0; fd < MAX_FD; fd++) {
        if (!file_table[fd].used) {
            struct ext2_inode tmp_inode;
            /* 先读 inode 原始数据 */
            ext2_read_inode(ino, &tmp_inode);

            file_table[fd].used  = 1;
            file_table[fd].ino   = ino;
            file_table[fd].pos   = 0;
            /* 存下完整 inode 以备后续读取块时使用 */
            file_table[fd].inode = tmp_inode;

            /* 合并 i_size_lo 和 i_size_hi */
            uint64_t lo = tmp_inode.i_size_lo;
            uint64_t hi = tmp_inode.i_size_hi;
            file_table[fd].size = lo | (hi << 32);

            return fd;
        }
    }

    return -1;  /* 没有空闲槽 */
}

/**
 * 从 fd 对应的文件当前位置读取最多 count 字节到 buf，
 * 返回实际读到的字节数（可能 < count），出错返回 -1
 */
size_t ext2_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD || !file_table[fd].used)
        return -1;

    ext2_file_t *f = &file_table[fd];
    size_t to_read   = count;
    size_t total_r   = 0;
    const struct ext2_inode *inode = &f->inode;
    const struct ext2_super_block *sb = ext2_sb();
    uint32_t block_size = 1024U << sb->s_log_block_size;

    uint8_t *tmp = kmalloc(block_size);
    if (!tmp) return -1;

    while (to_read > 0 && f->pos < f->size) {
        uint32_t blk_idx    = f->pos / block_size;
        uint32_t blk_offset = f->pos % block_size;
        uint32_t blk;
        /* 仅支持直接块 */
        if (blk_idx < 12) {
            blk = inode->i_block[blk_idx];
        } else {
            break;  /* 超过直接块，不再支持 */
        }
        if (!blk) break;

        if (ext2_read_block(blk, tmp) < 0) break;

        /* 计算本次拷贝长度 */
        size_t chunk = block_size - blk_offset;
        if (chunk > to_read)       chunk = to_read;
        if (chunk > f->size - f->pos) chunk = f->size - f->pos;

        memcpy((uint8_t*)buf + total_r, tmp + blk_offset, chunk);
        f->pos      += chunk;
        total_r     += chunk;
        to_read     -= chunk;
    }

    kfree(tmp);
    return total_r;
}

/**
 * 关闭 fd，使该槽可重用
 */
int ext2_close(int fd) {
    if (fd < 0 || fd >= MAX_FD || !file_table[fd].used)
        return -1;
    file_table[fd].used = 0;
    return 0;
}

void ext2_selftest(void) {
    const char *paths[] = {
        "/hello.txt",
        "/subdir/bar.dat",
    };

    // 2. 依次打开、读取并打印两个测试文件
    for (int i = 0; i < (int)(sizeof(paths)/sizeof(paths[0])); i++) {
        const char *path = paths[i];
        int fd = ext2_open(path);
        if (fd < 0) {
            printf("open \"%s\" failed.\n", path);
            continue;
        }
        printf("open \"%s\" => fd=%d\n", path, fd);

        // 假设每个文件内容不超过 128 字节
        char buf[129];
        size_t n = ext2_read(fd, buf, sizeof(buf)-1);
        if ((int)n < 0) {
            printf("read \"%s\" failed.\n", path);
        } else {
            buf[n] = '\0';
            printf("read \"%s\" => %u bytes: \"%s\"\n",
                   path, n, buf);
        }

        ext2_close(fd);
        printf("closed fd=%d\n\n", fd);
    }
}