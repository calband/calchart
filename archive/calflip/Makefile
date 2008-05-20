CC = gcc
LINK = gcc
CFLAGS = -Wall -m486 -O2
LFLAGS =

OBJS = calflip.o
OBJS2 = calflipspr.o
SOURCES = calflip.c calchart.h

.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $*.o

all: calflip calflipspr

calflip: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LIBS) -o calflip

calflipspr: $(OBJS2)
	$(CC) $(LFLAGS) $(OBJS2) $(LIBS) -o calflipspr

TAGS: $(SOURCES)
	etags $(SOURCES)

calflip.o: calflip.c calchart.h
