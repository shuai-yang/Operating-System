KERNEL_SOURCE=/lib/modules/`uname -r`/build

all: lexus test-lexus

lexus:
	 make -C ${KERNEL_SOURCE} M=`pwd` modules

obj-m += lexus.o

test-lexus: test-lexus.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	/bin/rm -f test-lexus test-lexus.o
	/bin/rm -f .lexus* *.o *.mod.c *.mod.o *.ko Module.symvers modules.order
	/bin/rm -rf .tmp_versions/
