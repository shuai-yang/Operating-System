KERNEL_SOURCE=/lib/modules/$(shell uname -r)/build
MY_CFLAGS += -g -DDEBUG -O0
ccflags-y += ${MY_CFLAGS}
CC += ${MY_CFLAGS}

obj-m += infiniti.o

# turns out that your module can't have the same name as your main file, or they call it a circular dependency issue;
# at least this is true if you have multiple source files, see kvm in Linux kernel for example.
infiniti-objs += fault.o infiniti_main.o

all:
	make -C ${KERNEL_SOURCE} M=`pwd` modules
	EXTRA_CFLAGS="$(MY_CFLAGS)"
clean:
	make -C $(KERNEL_SOURCE) M=$(PWD) clean
	/bin/rm -rf .tmp_versions/

.PHONY: all clean
