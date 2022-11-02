CC=gcc
AR = ar
#LD = ld
CFLAGS=-Wall -MMD -g -O0 -fPIC -Wpointer-arith -std=gnu89 -MP

LIBOBJS = cthreads.a

OBJS =  cthreads-test1 cthreads-test2 cthreads-test3\
	cthreads-test4 cthreads-test5 cthreads-test6\
	cthreads-test7 cthreads-test8\

#cthreads.so: $(LIBOBJS)
#	$(LD) -shared -o $@  $(LIBOBJS)

#cthreads.a: $(LIBOBJS)
#	$(AR)  rcv $@ $(LIBOBJS)
#	ranlib $@

#cthreads-test1: cthreads-test1.o cthreads.a
#	$(CC) $(CFLAGS) -o $@ $?

#cthreads-test2: cthreads-test2.o cthreads.a
#	$(CC) $(CFLAGS) -o $@ $?

#cthreads-test3: cthreads-test3.o cthreads.a
#	$(CC) $(CFLAGS) -o $@ $?

#cthreads-test4: cthreads-test4.o cthreads.a
#	$(CC) $(CFLAGS) -o $@ $?

#cthreads-test5: cthreads-test5.o cthreads.a
#	$(CC) $(CFLAGS) -o $@ $?

build = \
	@if [ -z "$V" ]; then \
		echo '  [$1]    $@'; \
		$2; \
	else \
		echo '$2'; \
		$2; \
	fi

% : %.c $(LIBOBJS)
	$(call build,LINK,$(CC) $(CFLAGS) $< $(LIBOBJS) -o $@)

%.o : %.c
	$(call build,CC,$(CC) $(CFLAGS) -c $< -o $@)

#%.so: %.o
#	$(call build,LD,$(LD) -shared -o $@ $^)

%.a : %.o
	$(call build,AR,$(AR) rcs $@ $^)

all: $(LIBOBJS) $(OBJS)

clean:
	rm -rf *.o a.out *.so *.a *.d
	rm -f *.o *.d $(OBJS) $(LIBOBJS)
