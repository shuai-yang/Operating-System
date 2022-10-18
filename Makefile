# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
INCLUDE=include/
CFLAGS = -Wall -I$(INCLUDE) -I./list/include/
OBJS = server.o client.o
LIBS=-lpthread

.SUFFIXES: .c .o 

all: concurrent_server server client spin.cgi

concurrent_server: concurrent_server.o Item.o list/lib/libmylib.a
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) concurrent_server server client spin.cgi *.o
