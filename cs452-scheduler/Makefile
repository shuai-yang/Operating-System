KERNEL_SOURCE=/lib/modules/`uname -r`/build
MY_CFLAGS += -g -DDEBUG -O0
ccflags-y += ${MY_CFLAGS}
CC += ${MY_CFLAGS}

all: lexus test-lexus

debug:
	make -C ${KERNEL_SOURCE} M=`pwd` modules
	EXTRA_CFLAGS="$(MY_CFLAGS)"

lexus:
	make -C ${KERNEL_SOURCE} M=`pwd` modules

obj-m += lexus.o

test-lexus: test-lexus.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	/bin/rm -f test-lexus test-lexus.o
	/bin/rm -f .lexus* *.o *.mod.c *.mod.o *.ko Module.symvers modules.order
	/bin/rm -rf .tmp_versions/
