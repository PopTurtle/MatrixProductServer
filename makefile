shared_dir = ./shared/
requests_dir = ./requests/

dirs = $(shared_dir) $(requests_dir)

CC = gcc

CPPFLAGS = -D_POSIX_SOURCE -D_FORTIFY_SOURCE=2

# fpie: a retirer ?
CFLAGS = -std=c2x \
	-Wall -Wextra -Wpedantic \
	-fstack-protector-all -fpie \
	-I$(shared_dir) -I$(requests_dir)

# a retirer ?
LDFLAGS = -Wl,-z,relro,-z,now -pie

vpath %.c $(dirs)
vpath %.h $(dirs)

.PHONY: all clean

all: server client

clean:
	rm *.o
	rm server
	rm client


server: server.o communication.o requests_management.o matrix.o
	$(CC) $(CFLAGS) $^ -o $@

client: client.o communication.o
	$(CC) $(CFLAGS) $^ -o $@


server.o: server.c communication.h
client.o: client.c communication.h
communication.o: communication.c communication.h
requests_management.o: requests_management.c communication.h matrix.h
matrix.o: matrix.c
