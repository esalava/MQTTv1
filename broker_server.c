#include "csapp.h"
#include <pthread.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include "common.h"

void forward_message(int connfd);
void *thread(void *vargp);

int conn_lst[2];
int conn_ind = 0;

pthread_t thread_id;

int main(int argc, char **argv)
{
	
	//Sockets
	int listenfd, *connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	//struct hostent *hp;
	char *port;
	

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	port = argv[1];
	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return 1;
	}

	printf("Waiting for connections...\n");
	listenfd = Open_listenfd(port);

	
	printf("server escuchando en puerto %s...\n", port);

	if(listenfd < 0)
		exit(-1);


	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		
		pthread_create(&thread_id, NULL, thread, connfd);

	}

}

void forward_message(int connfd)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio_src;
	rio_t rio_dst;

	while(conn_lst[1] != 5);
	
	int publisher = conn_lst[0];
	int suscriber = conn_lst[1];

	Rio_readinitb(&rio_src, publisher);
	Rio_readinitb(&rio_dst, suscriber);

	while((n = Rio_readlineb(&rio_src, buf, MAXLINE))) {
		if (strcmp(buf,"help\n")==0)
			Rio_writen(suscriber, "NO HELP FOR YOU\n", 16);
		else{
			Rio_writen(suscriber, buf, n);
			buf[strlen(buf)-1] = '\0';
			printf("server received %lu bytes (%s)\n", n,buf);
			
		}
	}
}


void *thread(void *vargp)
{
	int connfd = *((int *) vargp);
	//se agrega variables de reevio
	conn_lst[conn_ind++] = connfd;  //poner semaforos
	printf("Socket: %d\n",connfd);

	pthread_detach(pthread_self());
	free(vargp);
	forward_message(connfd);
	close(connfd);
	return NULL;
}
