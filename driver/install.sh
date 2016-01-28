	mknod /dev/MSRdrv c 249 0
	chmod 666 /dev/MSRdrv
	insmod -f MSRdrv.ko
