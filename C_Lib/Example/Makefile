INCLUDE_DIR =../Include
LIB_DIR=../Library
CC=gcc
CFLAGS=-L$(LIB_DIR) -I$(INCLUDE_DIR) -lpru_bridge
all: read write stream_read stream_write onebyte_time fourbyte_time

read: read.c
	$(CC) read.c -o read $(CFLAGS) 

write: write.c
	$(CC) write.c -o write $(CFLAGS)

stream_read: stream_read.c
	$(CC) stream_read.c -o stream_read $(CFLAGS)

stream_write: stream_write.c
	$(CC) stream_write.c -o stream_write $(CFLAGS)

onebyte_time: onebyte_time.c
	$(CC) onebyte_time.c -o onebyte_time $(CFLAGS)

fourbyte_time: fourbyte_time.c
	$(CC) fourbyte_time.c -o fourbyte_time $(CFLAGS)

clean:
	rm -f read write stream_read stream_write onebyte_time fourbyte_time

