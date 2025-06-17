#!/bin/sh
set -e
. ./iso.sh

# qemu-system-$(./target-triplet-to-arch.sh $HOST) -s -S -cdrom myos.iso
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso

