.PHONY: all

CC = gcc
LFLAGS = -lm
CFLAGS = -g -std=c99

all: 
	$(CC) $(CFLAGS) -o giffixer src/linkedlist.c src/lzwdecoder.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -o encoder src/linkedlist.c src/lzwencoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -o decoder src/lzwdecoder.c src/lzwdecodertest.c $(LFLAGS)

debug:
	$(CC) $(CFLAGS) -DDEBUG=1 -o giffixer src/linkedlist.c src/lzwdecoder.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -DDEBUG=1 -o encoder src/linkedlist.c src/lzwencoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -DDEBUG=1 -o decoder src/lzwdecoder.c src/lzwdecodertest.c $(LFLAGS)

memcheck:
	make debug
	valgrind --tool=memcheck --leak-check=yes -v ./decoder

clean:
	$(RM) giffixer encoder decoder
