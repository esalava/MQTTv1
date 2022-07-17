#include "csapp.h"
#include <unistd.h>

char *host = "127.0.0.1";
char *port = "8090";

int main(int argc, char **argv)
{
	int clientfd;
	char *node_name;
	char *pub_info;

	char node_name_buf[20];
	char pub_info_buf[20]; // = "7,12,42\n";
	char buf_suscription[] = "NONE\n";
	
	rio_t rio;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <node_name> <info>\n", argv[0]);
		exit(0);
	}
	
	node_name = argv[1];
	pub_info = argv[2];

	strcpy(node_name_buf, node_name);
    strcpy(pub_info_buf, pub_info);

	strcat(node_name_buf, "\n");
	strcat(pub_info_buf, "\n");


	clientfd = Open_clientfd(host, port);


	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, node_name_buf, strlen(node_name_buf));
	Rio_writen(clientfd, buf_suscription, strlen(buf_suscription));

	printf("Name: %sSuscription: %sConnected to: %s\n\n\n", node_name_buf, buf_suscription,host);

	

	while (1) {
		sleep(2);
		Rio_writen(clientfd, pub_info_buf, strlen(pub_info_buf));
		printf("Se ha enviado la informacion de estado...\n");
	}

	Close(clientfd);
	exit(0);
}
