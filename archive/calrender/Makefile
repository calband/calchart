CC = cc
CFLAGS = -O2 -m486 -Wall
LIBS =
SRCS = calrender.c
OBJS = $(SRCS:.c=.o)
PROGRAM = calrender

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)

.c.o:
	$(CC) $(CFLAGS) -c $*.c
