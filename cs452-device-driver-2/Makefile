KERNEL_SOURCE=/lib/modules/`uname -r`/build
MY_CFLAGS += -g -DDEBUG -O0 -Wno-overflow
ccflags-y += ${MY_CFLAGS}
CC += ${MY_CFLAGS}

debug:
	make -C ${KERNEL_SOURCE} M=`pwd` modules
	EXTRA_CFLAGS="$(MY_CFLAGS)"

all:
	make -C ${KERNEL_SOURCE} M=`pwd` modules

obj-m = lincoln.o

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm -f .lincoln* lincoln.o lincoln.mod.c lincoln.mod.o lincoln.ko Module.symvers modules.order
	rm -rf .tmp_versions/
