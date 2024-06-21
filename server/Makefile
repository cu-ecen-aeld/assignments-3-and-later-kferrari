NAME = aesdsocket

CC=$(CROSS_COMPILE)gcc

all: default

default: aesdsocket.c
	$(CC) -g -Wall -c -o aesdsocket.o aesdsocket.c
	$(CC) -g -Wall -I/ aesdsocket.o -o aesdsocket

clean: 
	rm -f *.bin *.o aesdsocket