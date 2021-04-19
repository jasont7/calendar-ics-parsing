#
# "makefile" for the third implementation of "icsout" (icsout3)
# Spring 2021, assignment #3)
#

CC=gcc

# The line with -DDEBUG can be used for development. When
# building your code for evaluation, however, the line *without*
# the -DDEBUG will be used.
#
#CFLAGS=-c -Wall -g -DDEBUG -std=c99
CFLAGS=-c -Wall -g -std=c99


all: icsout3

icsout3: icsout3.o listy.o emalloc.o
	$(CC) icsout3.o listy.o emalloc.o -o icsout3

icsout3.o: icsout3.c listy.h ics.h emalloc.h
	$(CC) $(CFLAGS) icsout3.c

listy.o: listy.c listy.h ics.h emalloc.h
	$(CC) $(CFLAGS) listy.c

emalloc.o: emalloc.c emalloc.h
	$(CC) $(CFLAGS) emalloc.c

clean:
	rm -rf *.o icsout3 
