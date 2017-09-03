CC = gcc

CFLAGS = -Wall -pedantic -std=c89 -g -lpthread 
CURSES = -lmenuw -lncursesw
all: fm.o
	${CC} ${CFLAGS} ${CURSES} fm.o -o fm
fm.o: fm.c
	${CC} ${CFLAGS} -c fm.c
clean:
	rm *.o fm

