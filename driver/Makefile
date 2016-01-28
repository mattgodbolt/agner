KERNELDIR := /lib/modules/`uname -r`/build
obj-m := MSRdrv.o
default:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules
clean:
	rm -f .MSRdrv.*
	rm -f -r .tmp_versions
	rm -f *~
	rm -f MSRdrv.ko
	rm -f MSRdrv.o
	rm -f MSRdrv.mod.*
	rm -f linux.mdl
	rm -f Modules.symvers

