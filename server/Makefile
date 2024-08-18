all: aesdsocket
default: aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -pthread -lrt -Wall -c aesdsocket.c

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -pthread -lrt -Wall -o aesdsocket

clean :
	rm -f aesdsocket.o aesdsocket