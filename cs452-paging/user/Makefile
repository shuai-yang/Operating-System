CC = gcc
AR = ar
CFLAGS = 

LIB_OBJS = harness.a

OBJS =  infiniti-test1 infiniti-test2 infiniti-test3\
	infiniti-test4 infiniti-test5 infiniti-test6\

build = \
	@if [ -z "$V" ]; then \
		echo '	[$1]	$@'; \
		$2; \
	else \
		echo '$2'; \
		$2; \
	fi

% : %.c $(LIB_OBJS)
	$(call build,LINK,$(CC) $(CFLAGS) $< $(LIB_OBJS) -o $@)

%.o : %.c 
	$(call build,CC,$(CC) $(CFLAGS) -c $< -o $@)

%.a : %.o
	$(call build,AR,$(AR) rcs $@ $^)


all: $(LIB_OBJS) $(OBJS)


clean: 
	rm -f *.o $(OBJS) $(LIB_OBJS)
