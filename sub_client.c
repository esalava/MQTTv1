#include "csapp.h"
#include <unistd.h>


int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host;
	rio_t rio;

	/*Node information*/
	char buf[MAXLINE] = "7%\n";
	char buf_node_name[] = "node2\n";
	char buf_suscription[] = "node1\n";
	
	

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	
	host = argv[1];
	port = argv[2];


	clientfd = Open_clientfd(host, port);

	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, buf_node_name, strlen(buf_node_name));
	Rio_writen(clientfd, buf_suscription, strlen(buf_suscription));
	printf("Name: %sSuscription: %sConnected to: %s\n\n\n", buf_node_name, buf_suscription,host);
	
	while (1) {
		if(Rio_readlineb(&rio, buf, MAXLINE)){
            Fputs(buf, stdout);
        }    
	}


	Close(clientfd);
	exit(0);
}
