CC = gcc
CFLAGS = -Wall -pedantic -std=c89 -g
CURSES = -lncursesw
all: editor.o
	${CC} ${CFLAGS} ${CURSES} editor.o -o editor
editor.o: editor.c
	${CC} ${CFLAGS} -c editor.c
clean:
	rm *.o editor

