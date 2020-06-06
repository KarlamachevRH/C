#!/bin/bash

arm-linux-gnueabihf-gcc -static hello.c -o init
echo init | cpio -o -H newc | gzip > ../initramfs.cpio.gz