#include <stdint.h>
#include <stdio.h>
#include "kernel/io.h"
#include "kernel/ata.h"

#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_ERROR      0x1F1
#define ATA_PRIMARY_SECCOUNT   0x1F2
#define ATA_PRIMARY_LBA_LOW    0x1F3
#define ATA_PRIMARY_LBA_MID    0x1F4
#define ATA_PRIMARY_LBA_HIGH   0x1F5
#define ATA_PRIMARY_DRIVE      0x1F6
#define ATA_PRIMARY_STATUS     0x1F7
#define ATA_PRIMARY_COMMAND    0x1F7

// 读 1 扇区到 buf（512 字节），lba 从 0 开始
static void ata_read_sector(uint32_t lba, uint8_t *buf) {
    // 1. 选择驱动 + LBA 模式
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();

    // 2. 设定要读的扇区数和 LBA 地址低 24 位
    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW,  (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_LBA_MID,  (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));

    // 3. 发出读扇区命令（0x20 = READ SECTORS）
    outb(ATA_PRIMARY_COMMAND, 0x20);

    // 4. 等待 BSY 清零并且 DRQ 被置位
    while (inb(ATA_PRIMARY_STATUS) & 0x80) { /* BSY */ }
    while (!(inb(ATA_PRIMARY_STATUS) & 0x08)) { /* DRQ */ }

    // 5. 数据端口一次 16 位读取 256 次
    for (int i = 0; i < 256; i++) {
        uint16_t w = inw(ATA_PRIMARY_DATA);
        buf[i*2 + 0] = w & 0xFF;
        buf[i*2 + 1] = w >> 8;
    }
}

// 在 kernel_main 或初始化早期调用
#define EXT2_SUPERBLOCK_SECTOR  (1024 / 512)    // = 2
#define EXT2_MAGIC_OFFSET       56             // 在 superblock 中的偏移

void disk_test() {
    uint8_t buf[512];
    ata_read_sector(EXT2_SUPERBLOCK_SECTOR, buf);

    uint16_t magic = *(uint16_t*)&buf[EXT2_MAGIC_OFFSET];
    if (magic == 0xEF53) {
        printf("EXT2 test OK: magic 0xEF53 found\n");
    } else {
        printf("EXT2 test FAILED: found 0x%x\n", magic);
    }
}
