CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: client server

client: echoclient.c csapp.o
	$(CC) $(CFLAGS) -o client echoclient.c csapp.o $(LIB)

server: echoserveri.c csapp.o
	$(CC) $(CFLAGS) -o server echoserveri.c csapp.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o client server *~

