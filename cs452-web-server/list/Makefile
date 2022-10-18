
INSTALL=.
INCLUDES=$(INSTALL)/include -I.
LIBS=$(INSTALL)/lib
LFLAGS=-L$(LIBS) -lmylib
CC=gcc
CFLAGS=-Wall -g  -I$(INCLUDES)

PROGS=SimpleTestList 

all: $(PROGS)


SimpleTestList: SimpleTestList.o
	$(CC) $(CFLAGS) -o $@ SimpleTestList.o   $(LFLAGS)


clean:
	rm -f list *.o $(PROGS)
