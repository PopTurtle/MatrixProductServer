CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
exec = server

all: $(exec)

clean:
	rm *.o
	rm $(exec)

$(exec): main.o
	$(CC) $(CFLAGS) main.o -o $(exec)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
