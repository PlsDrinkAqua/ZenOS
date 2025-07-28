#ifndef _ATA_H
#define _ATA_H

#include<stdint.h>

typedef struct {
  uint32_t (*read)(uint32_t lba, uint8_t count, uint8_t *buf);
  uint32_t (*write)(uint32_t lba, uint8_t count, const uint8_t *buf);
  uint32_t block_size;
  uint32_t total_blocks;
} block_device_t;

uint32_t ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer);

uint32_t ata_write_sectors(uint32_t lba, uint8_t count, const uint8_t *buffer);

void ata_rw_selftest(void);

void block_devices_init(void);

#endif