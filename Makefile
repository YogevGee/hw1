# Makefile for hw1shell

CC = gcc
CFLAGS = -Wall -g
OBJ = main.o exec_utils.o

all: hw1shell

hw1shell: $(OBJ)
	$(CC) $(CFLAGS) -o hw1shell $(OBJ)

main.o: main.c exec_utils.h
	$(CC) $(CFLAGS) -c main.c

exec_utils.o: exec_utils.c exec_utils.h
	$(CC) $(CFLAGS) -c exec_utils.c

clean:
	rm -f *.o hw1shell
