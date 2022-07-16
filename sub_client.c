#include "csapp.h"
#include <unistd.h>


int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host;
	char buf[MAXLINE] = "7%\n";
	char buf_suscription[] = "node1/cpu_usage\n";
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
    
	Rio_writen(clientfd, buf_suscription, strlen(buf_suscription));

	while (1) {
		if(Rio_readlineb(&rio, buf, MAXLINE)){
            Fputs(buf, stdout);
        }    
	}


	Close(clientfd);
	exit(0);
}
