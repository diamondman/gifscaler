.PHONY: all

CC = gcc
LFLAGS = -lm
CFLAGS = -g

all: 
	$(CC) $(CFLAGS) -o giffixer src/linkedlist.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -o encoder src/linkedlist.c src/lzwencoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -o decoder src/linkedlist.c src/lzwdecoder.c $(LFLAGS)


clean:
	$(RM) giffixer encoder decoder