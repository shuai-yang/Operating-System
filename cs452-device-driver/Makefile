KERNEL_SOURCE=/lib/modules/`uname -r`/build
CC = gcc

all: toyota toyota-test1 toyota-test2 toyota-test3 toyota-test4

toyota:
	make -C ${KERNEL_SOURCE} M=`pwd` modules

obj-m += toyota.o

toyota-test1: toyota-test1.o
	$(CC) $(CFLAGS) -o $@ $^

toyota-test2: toyota-test2.o
	$(CC) $(CFLAGS) -o $@ $^

toyota-test3: toyota-test3.o
	$(CC) $(CFLAGS) -o $@ $^

toyota-test4: toyota-test4.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	/bin/rm -f toyota-test1 toyota-test2 toyota-test3 toyota-test4 toyota-test*.o
	/bin/rm -f .toyota* toyota.o toyota.mod.c toyota.mod.o toyota.ko Module.* modules.*
	/bin/rm -rf .tmp_versions/
