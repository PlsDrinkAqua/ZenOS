#!/bin/bash
set -e

# 1. 先创建一个基础镜像 ext2_hda.img
dd if=/dev/zero of=ext2_hda.img bs=1M count=100          # 100 MiB 空盘
mkfs.ext2 ext2_hda.img                                  # 在整个文件上格式化 ext2

# 2. 复制出更多硬盘
# for letter in b c d; do
#   cp ext2_hda.img ext2_hd${letter}.img
# done

cp ./ext2_hda.img disk/ext2_hda.img
echo "Generation Completed：ext2_hda.img"
