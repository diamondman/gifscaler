.PHONY: all

CC = gcc
LFLAGS = -lm
CFLAGS = -g
SOURCE = src/linkedlist.c src/main.c

all: 
	$(CC) $(CFLAGS) -o giffixer $(SOURCE) $(LFLAGS)


#clean:
#	$(RM) $()
#o	$(RM) -r *.dSYM