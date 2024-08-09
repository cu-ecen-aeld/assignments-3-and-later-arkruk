all: aesdsocket
default: aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -pthread -c aesdsocket.c

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -pthread -o aesdsocket

clean :
	rm -f aesdsocket.o aesdsocket