#!/bin/bash

QEMU_AUDIO_DRV=none qemu-system-arm -M vexpress-a9 -nographic -kernel zImage \
-initrd initramfs.cpio.gz -dtb vexpress-v2p-ca9.dtb -append "console=ttyAMA0"