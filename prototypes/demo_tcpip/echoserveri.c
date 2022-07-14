#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv)
{
	int listenfd, connfd;
	unsigned int clientlen;
	struct sockaddr_in clientaddr;
	struct hostent *hp;
	char *haddrp, *port;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	port = argv[1];

	printf("Waiting for connections...\n");
	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

		/* Determine the domain name and IP address of the client */
		hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
					sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		haddrp = inet_ntoa(clientaddr.sin_addr);
		
		if (hp)
			printf("\n>> Server connected to %s (%s)\n\n", hp->h_name, haddrp);
		else
			printf("server connected to %s\n", haddrp);
		
		echo(connfd);
		Close(connfd);
		printf("\nServer disconnected from %s\n\n", haddrp);
	}
	exit(0);
}

void echo(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio;

	Rio_readinitb(&rio, connfd);
	while((n = Rio_readlineb(&rio, buf, MAXLINE))) {
		if (strcmp(buf,"help\n")==0)
			Rio_writen(connfd, "NO HELP FOR YOU\n", 16);
		else{
			Rio_writen(connfd, buf, n);
			buf[strlen(buf)-1] = '\0';
			printf("server received %lu bytes (%s)\n", n,buf);
			
		}
	}
}
