.PHONY: all

CC = gcc
LFLAGS = -lm
CFLAGS = -g -std=c99

all: 
	$(CC) $(CFLAGS) -o giffixer src/linkedlist.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -o encoder src/linkedlist.c src/lzwencoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -o decoder src/lzwdecoder.c $(LFLAGS)

debug:
	$(CC) $(CFLAGS) -DDEBUG=1 -o giffixer src/linkedlist.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -DDEBUG=1 -o encoder src/linkedlist.c src/lzwencoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -DDEBUG=1 -o decoder src/lzwdecoder.c $(LFLAGS)

memcheck:
	make debug
	valgrind --tool=memcheck --leak-check=yes -v ./encoder

clean:
	$(RM) giffixer encoder decoder
