INCLUDE_DIR =../Include
LIB_DIR=../Library
CC=gcc
CFLAGS=-L$(LIB_DIR) -I$(INCLUDE_DIR) -lpru_bridge
all: read

read: read.c
	$(CC) read.c -o read $(CFLAGS) 

clean:
	rm -f read 

