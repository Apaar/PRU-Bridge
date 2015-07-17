INCLUDE_DIR =../Include
LIB_DIR=../Library
CC=gcc
CFLAGS=-L$(LIB_DIR) -I$(INCLUDE_DIR) -lpru_bridge
all: read stream_read test_pointer

read: read.c
	$(CC) read.c -o read $(CFLAGS) 

stream_read: stream_read.c
	$(CC) stream_read.c -o stream_read $(CFLAGS)

test_pointer: test_pointer.c
	$(CC) test_pointer.c -o test_pointer $(CFLAGS)

clean:
	rm -f read stream_read test_pointer

