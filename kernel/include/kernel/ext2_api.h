#ifndef _EXT2_API_H
#define _EXT2_API_H

#include <stdint.h>
#include "kernel/ext2.h"

typedef struct {
    uint32_t ino;
    uint32_t pos;
    uint64_t size;
    struct ext2_inode inode;
    int used;
} ext2_file_t;

// 目录项 v2（带 file_type 字段）
struct ext2_dir_entry_2 {
    uint32_t inode;       // 本条目的 inode 号（0 表示空条目）
    uint16_t rec_len;     // 本条目长度（跳到下一个目录项）
    uint8_t  name_len;    // 文件名长度
    uint8_t  file_type;   // 文件类型，见下 enum
    char     name[];      // 文件名内容（不是 NUL 结尾，长度 = name_len）
};

// 文件类型常量（file_type 字段）
enum ext2_file_type {
    EXT2_FT_UNKNOWN   = 0,
    EXT2_FT_REG_FILE  = 1,
    EXT2_FT_DIR       = 2,
    EXT2_FT_CHRDEV    = 3,
    EXT2_FT_BLKDEV    = 4,
    EXT2_FT_FIFO      = 5,
    EXT2_FT_SOCK      = 6,
    EXT2_FT_SYMLINK   = 7,
};

int ext2_init(void);
void ext2_selftest(void);

#endif