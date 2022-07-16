#include "csapp.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	int clientfd;
	char *port;
	char *host;
	char pub_info_buf[MAXLINE] = "9,12,42\n";
	char buf_node_name[] = "node1\n";
	char buf_suscription[] = "NONE\n";
	

	rio_t rio;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	
	host = argv[1];
	port = argv[2];


	clientfd = Open_clientfd(host, port);
	//printf("Succesfully connected to %s\n",host);

	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, buf_node_name, strlen(buf_node_name));
	Rio_writen(clientfd, buf_suscription, strlen(buf_suscription));

	//buf_node_name[strlen(buf_node_name) - 1] = '\0';
	//buf_suscription[strlen(buf_suscription) - 1] = '\0';

	printf("Name: %sSuscription: %sConnected to: %s\n\n\n", buf_node_name, buf_suscription,host);

	

	while (1) {
		sleep(5);
		Rio_writen(clientfd, pub_info_buf, strlen(pub_info_buf));
		printf("Se ha enviado la informacion de estado...\n");
	}

	Close(clientfd);
	exit(0);
}
