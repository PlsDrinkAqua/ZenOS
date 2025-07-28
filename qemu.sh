#!/bin/sh
set -e
. ./disk.sh
. ./iso.sh

# qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -cdrom myos.iso \
#     -hda ext2_hda.img
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso \
    -hda ext2_hda.img

