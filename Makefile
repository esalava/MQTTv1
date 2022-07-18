CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: broker publisher suscriber exec_top

broker: broker_server.c csapp.o
	$(CC) $(CFLAGS) -o broker broker_server.c csapp.o $(LIB)

publisher: pub_client.c csapp.o
	$(CC) $(CFLAGS) -o publisher pub_client.c csapp.o $(LIB)

suscriber: sub_client.c csapp.o
	$(CC) $(CFLAGS) -o suscriber sub_client.c csapp.o $(LIB)

exec_top: exec_top.c 
	$(CC) $(CFLAGS) -o exec_top exec_top.c 

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f *.o broker broker suscriber *~