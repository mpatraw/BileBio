
all: bilebio

clean:
	rm bilebio.o bilebio

bilebio: bilebio.o
	gcc bilebio.o -o bilebio -lm -lcurses -ltcod -Wl,-rpath '-Wl,$ORIGIN'

bilebio.o: bilebio.c
	gcc -c -g -pedantic -std=c99 -Wall -Wextra bilebio.c

