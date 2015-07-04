INCLUDE_DIR =../Include
LIB_DIR=../Library
CC=gcc
CFLAGS=-L$(LIB_DIR) -I$(INCLUDE_DIR) -lpru_bridge
all: read stream_read

read: read.c
	$(CC) read.c -o read $(CFLAGS) 

stream_read: stream_read.c
	$(CC) stream_read.c -o stream_read $(CFLAGS)

clean:
	rm -f read stream_read

