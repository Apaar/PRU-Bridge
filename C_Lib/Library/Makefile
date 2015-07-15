INCLUDE_DIR =../Include
CC=gcc
CFLAGS=-c -I$(INCLUDE_DIR)

libpru_bridge.a: libpru_bridge.o
	ar -rcs libpru_bridge.a libpru_bridge.o

libpru_bridge.o: libpru_bridge.c
	$(CC) $(CFLAGS) libpru_bridge.c

clean:
	rm -f *.o *.a
