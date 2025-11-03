#!/bin/bash

rm -f /dev/MSRdrv
mknod /dev/MSRdrv c 249 0
chmod 666 /dev/MSRdrv
insmod MSRdrv.ko
