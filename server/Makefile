all: aesdsocket
default: aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -pthread -lrt -c aesdsocket.c

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -pthread -lrt -o aesdsocket

clean :
	rm -f aesdsocket.o aesdsocket