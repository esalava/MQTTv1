CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: broker publisher

broker: broker_server.c csapp.o
	$(CC) $(CFLAGS) -o broker broker_server.c csapp.o $(LIB)

publisher: pub_client.c csapp.o
	$(CC) $(CFLAGS) -o publisher pub_client.c csapp.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o client server *~