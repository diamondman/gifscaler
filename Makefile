.PHONY: all

CC = gcc
LFLAGS = -lm
CFLAGS = -g -std=c99
DBG = -DDEBUG=1

all: 
	$(CC) $(CFLAGS) -o bin/giffixer src/linkedlist.c src/lzw_decoder.c src/gif_decoder.c src/gif_utils.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) -o bin/encoder src/linkedlist.c src/lzw_encoder.c src/test_lzw_encoder.c $(LFLAGS)
	$(CC) $(CFLAGS) -o bin/decoder src/lzw_decoder.c src/test_lzw_decoder.c $(LFLAGS)

debug:
	$(CC) $(CFLAGS) $(DBG) -o bin/giffixer src/linkedlist.c src/lzw_decoder.c src/gif_decoder.c src/gif_utils.c src/main.c $(LFLAGS)
	$(CC) $(CFLAGS) $(DBG) -o bin/encoder src/linkedlist.c src/lzw_encoder.c src/test_lzw_encoder.c $(LFLAGS)
	$(CC) $(CFLAGS) $(DBG) -o bin/decoder src/lzw_decoder.c src/test_lzw_decoder.c $(LFLAGS)

test: debug
	./bin/giffixer samples/grad.gif

memcheck: debug
	valgrind --tool=memcheck --leak-check=yes -v ./bin/giffixer samples/grad.gif

clean:
	rm bin/giffixer bin/encoder bin/decoder
