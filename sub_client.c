#include "csapp.h"
#include <unistd.h>
#include <string.h>
#include "MQTTconfig.h"

char *host = BROKER_HOST;//"127.0.0.1";
char *port = BROKER_PORT;//"8090";

int main(int argc, char **argv)
{
	int clientfd;
	char *node_name;
	char *suscription_topic;
	rio_t rio;

	/*Node information*/
	char buf[MAXLINE];
    char node_name_buf[20];
    char suscription_topic_buf[20];
	

	if (argc != 3) {
		fprintf(stderr, "usage: %s <node_name> <suscription_topic>\n", argv[0]);
		fprintf(stderr, "suscription_topic: <node>/<topic>\n");
		fprintf(stderr, "availables topics:\n");
		fprintf(stderr, " -cpu (shows node's cpu status)\n");
		fprintf(stderr, " -ram (shows node's ram status)\n");
		fprintf(stderr, " -tasks (shows node's tasks status)\n");
		exit(0);
	}
	
	/* se realiza formato de argumentos */
	node_name = argv[1];
	suscription_topic = argv[2];

    strcpy(node_name_buf, node_name);
    strcpy(suscription_topic_buf, suscription_topic);

	strcat(node_name_buf, "\n");
	strcat(suscription_topic_buf, "\n");

	printf("%s", node_name_buf);
	printf("%s", suscription_topic_buf);

	clientfd = Open_clientfd(host, port);

	/* fin de formato de argumentos */

	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, node_name_buf, strlen(node_name_buf));										//se envia el nombre del nodo al broker
	Rio_writen(clientfd, suscription_topic_buf, strlen(suscription_topic_buf));						//se envia el topico del cual se encuentra subscrito
	printf("Name: %s Suscription: %s Connected to: %s\n\n\n", node_name, suscription_topic, host);  //mensaje de control
	
	//se mantiene en constante escucha de lo mensajes que provengan del broker
	while (1) {
		if(Rio_readlineb(&rio, buf, MAXLINE)){
			printf("%s: ",suscription_topic);
            Fputs(buf, stdout);
        } else {
			break;
		}
	}

	Close(clientfd);
	exit(0);
}
