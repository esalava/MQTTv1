#include "csapp.h"
#include <unistd.h>


int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host;
	char buf[MAXLINE] = "CPU_USAGE 9%\n";
	rio_t rio;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	
	host = argv[1];
	port = argv[2];


	clientfd = Open_clientfd(host, port);
	Rio_readinitb(&rio, clientfd);
	printf("Succesfully connected to %s\n",host);

	while (1) {
		sleep(5);
		Rio_writen(clientfd, buf, strlen(buf));
		printf("Se ha enviado la informacion de estado...\n");
	}

	Close(clientfd);
	exit(0);
}
