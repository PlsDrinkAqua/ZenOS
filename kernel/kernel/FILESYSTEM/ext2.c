#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel/ata.h"
#include "kernel/ext2.h"
#include "kernel/kmalloc.h"
#include "kernel/vmm.h"

#define SECTOR_SIZE 512U
#define SUPER_SECTOR 2        // Superblock 从 LBA=2 开始
#define SUPER_READ_SECS 2     // 共 2 扇区 = 1024B
#define INODE_SIZE 128

/* 本文件私有 */
static struct ext2_super_block sb;
static struct ext2_group_desc *gbdt;
static uint32_t sb_groups_count;

static uint8_t read_sb(void) {
    static uint8_t buf[SECTOR_SIZE * SUPER_READ_SECS];
    if (!ata_read_sectors(SUPER_SECTOR, SUPER_READ_SECS, buf)) {
        printf("ext2: read superblock failed\n");
        return 0;
    }
    memcpy(&sb, buf, sizeof(sb));

    if (sb.s_magic != 0xEF53) {
        printf("Not an ext2 filesystem! magic=0x%x\n", sb.s_magic);
        return 0;
    }
    return 1;
}

/**
 * 从磁盘读取一个 inode 的完整内容
 *
 * @param inode_no   要读取的 inode 号 (1-based)
 * @param inode_out  调用者提供的缓冲区，大小至少 sb.s_inode_size
 * @return 0 on success, -1 on error
 */
int ext2_read_inode(uint32_t inode_no, struct ext2_inode *inode_out)
{
    // 只要 1 <= inode_no <= 总 inode 数就行
    if (inode_no == 0 || inode_no > sb.s_inodes_count)
        return -1;

    // inode_size 动态：版本 1+ 用 superblock 里的字段，否则固定 128
    uint32_t inode_size = (sb.s_rev_level >= 1 ? sb.s_inode_size : 128);

    /* 下面的逻辑不变：算出所在组 + 组内索引，算出哪个 block 和偏移 */
    uint32_t idx        = inode_no - 1;
    uint32_t group      = idx / sb.s_inodes_per_group;
    uint32_t index      = idx % sb.s_inodes_per_group;
    if (group >= sb_groups_count)
        return -1;

    uint32_t itable     = gbdt[group].bg_inode_table;
    uint32_t block_sz   = 1024U << sb.s_log_block_size;
    uint32_t byte_off   = index * inode_size;
    uint32_t block_off  = byte_off  / block_sz;
    uint32_t intra_off  = byte_off  % block_sz;

    uint32_t secs_pb    = block_sz / SECTOR_SIZE;
    uint32_t start_sec  = (itable + block_off) * secs_pb
                         + (intra_off / SECTOR_SIZE);
    uint32_t off_in_sec = intra_off % SECTOR_SIZE;
    uint32_t to_read    = (off_in_sec + inode_size + SECTOR_SIZE - 1)
                         / SECTOR_SIZE;

    /* 临时缓冲，读足扇区 */
    uint32_t tmp_bytes = to_read * SECTOR_SIZE;
    uint8_t *tmp = kmalloc(tmp_bytes);
    if (!tmp) return -1;
    if (!ata_read_sectors(start_sec, to_read, tmp)) {
        kfree(tmp);
        return -1;
    }

    /* 拷贝 inode_size 字节 */
    memcpy(inode_out, tmp + off_in_sec, INODE_SIZE);
    kfree(tmp);
    return 0;
}


int ext2_driver_init(void) {
    if (!read_sb())
        return -1;

    /* 1) 计算 Block Group 数量 */
    {
        uint32_t total_blocks     = sb.s_blocks_count;
        uint32_t first_data_block = sb.s_first_data_block;
        uint32_t bpg              = sb.s_blocks_per_group;
        sb_groups_count = (total_blocks - first_data_block + bpg - 1) / bpg;
    }

    /* 2) 读入 Group Descriptor Table */
    gbdt = kmalloc(sb_groups_count * sizeof(*gbdt));
    if (!gbdt) {
        printf("ext2_init: kmalloc for gbdt failed\n");
        return -1;
    }
    {
        uint32_t block_size      = 1024U << sb.s_log_block_size;
        uint32_t gd_block        = (block_size == 1024 ? 2 : 1);
        uint32_t secs_per_blk    = block_size / SECTOR_SIZE;
        uint32_t total_bytes     = sb_groups_count * sizeof(*gbdt);
        uint32_t secs_to_read    = (total_bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
        ata_read_sectors(gd_block * secs_per_blk,
                         secs_to_read,
                         (uint8_t*)gbdt);
    }

    /* 3) 打印一些信息 */
    // printf("Total Blocks:        %u\n", sb.s_blocks_count);
    // printf("Free Blocks:         %u\n", sb.s_free_blocks_count);
    // printf("Group 0 Free Blocks: %u\n", gbdt[0].bg_free_blocks_count);
    // printf("Ext2 version: %u.%u\n",
    //        sb.s_rev_level, sb.s_minor_rev_level);
    // if (sb.s_rev_level >= 1) {
    //     printf("Supports extended superblock fields\n");
    // } else {
    //     printf("Old revision (no extended fields)\n");
    // }

    /* 4) 读 root inode 并打印它的大小 */
    {
        const uint32_t ROOT_INO = 2;
        uint32_t isz = (sb.s_rev_level >= 1 ? sb.s_inode_size : 128);
        struct ext2_inode *root_inode = kmalloc(isz);
        if (!root_inode) {
            printf("ext2_init: kmalloc inode buf failed\n");
            return -1;
        }
        if (ext2_read_inode(ROOT_INO, root_inode) < 0) {
            printf("ext2_init: read root inode failed\n");
            kfree(root_inode);
            return -1;
        }

        // uint64_t size = (sb.s_rev_level >= 1)
        //     ? ((uint64_t)root_inode->i_size_hi << 32)
        //       | root_inode->i_size_lo
        //     : root_inode->i_size_lo;
        // printf("Root inode size: %u bytes\n",
        //        (unsigned long long)size);

        kfree(root_inode);
    }
    return 0;
}

const struct ext2_super_block *ext2_sb(void) {
  return &sb;
}
const struct ext2_group_desc *ext2_gbdt(void) {
  return gbdt;
}


// 计算每块包含多少扇区
static inline uint32_t ext2_sectors_per_block(void) {
    // sb 已经是模块私有的 static struct ext2_super_block
    return (1024U << sb.s_log_block_size) / SECTOR_SIZE;
}

/**
 * 读取整个数据块到 buf。
 * @param block_no  要读的块号（基于 0 的块索引）
 * @param buf       目标缓冲区，大小至少为 block_size
 * @return 0 成功，-1 失败
 */
int ext2_read_block(uint32_t block_no, void *buf) {
    uint32_t secs = ext2_sectors_per_block();
    uint32_t start_sec = block_no * secs;
    // ata_read_sectors 返回实际读到的扇区数
    if (ata_read_sectors(start_sec, secs, buf) != secs)
        return -1;
    return 0;
}