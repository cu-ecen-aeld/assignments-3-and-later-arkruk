all: aesdsocket
default: aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -c aesdsocket.c

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -o aesdsocket

clean :
	rm -f aesdsocket.o aesdsocket