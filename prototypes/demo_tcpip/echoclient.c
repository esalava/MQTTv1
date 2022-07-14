#include "csapp.h"

int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host, buf[MAXLINE];
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

	printf("Write something: ");
	while (Fgets(buf, MAXLINE, stdin) != NULL && strcmp(buf,"quit\n")!=0) {
		Rio_writen(clientfd, buf, strlen(buf));
		Rio_readlineb(&rio, buf, MAXLINE);
		Fputs(buf, stdout);
		printf("Write something: ");
	}
	Close(clientfd);
	exit(0);
}
