CROSS_COMPILE=gcc

default: writer

writer: writer.c
	$(CROSS_COMPILE) -c -o writer.o writer.c
	$(CROSS_COMPILE) -I/ writer.o -o writer

clean:
	rm -f *.o writer *.elf *.map
