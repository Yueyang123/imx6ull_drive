KERNELDIR :=/home/yuri/SDK/IMX6ULL/IMX6ULL_SOURCE/MYLINUX/KERNEL/Linux_imx_yuri
CURRENT_PATH := $(shell pwd)
obj-m := beep.o

build: kernel_modules test

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

test: test.o
	arm-linux-gnueabihf-gcc test.o -o test
test.o:test.c
	arm-linux-gnueabihf-gcc -o test.o -c test.c

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean