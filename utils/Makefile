CFLAGS = -O2 -Wall -Werror -I..

.PHONY: default clean

all: crctable

crctable: crctable.c ../crcfactory.h
	gcc $(CFLAGS) -o $@ $<

clean: 
	rm -f crctable
