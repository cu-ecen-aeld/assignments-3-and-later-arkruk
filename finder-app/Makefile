CROSS_COMPILE=gcc

default: writer

writer: writer.c
	$(CROSS_COMPILE) -Wall -c -o writer.o writer.c
	$(CROSS_COMPILE) -Wall -I/ writer.o -o writer

clean:
	rm -f *.o writer *.elf *.map
