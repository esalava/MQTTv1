#include "csapp.h"
#include <pthread.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include "common.h"
#include <semaphore.h>
#include <sys/shm.h>

#define SEM_CONN_CONTROL "/conn_control"
#define SEM_MUTEX "/mutex"

void forward_message(int connfd);
void *thread(void *vargp);
void split(char *linea, char *delim, char *argv[]);

int conn_lst[2];
int conn_ind = 0;

pthread_t thread_id;

sem_t *sem_control;
sem_t *sem_mutex;

int main(int argc, char **argv)
{
	
	//Sockets
	int listenfd, *connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	//struct hostent *hp;
	char *port;
	
	//set up semaphores
	sem_unlink(SEM_CONN_CONTROL);
	sem_unlink(SEM_MUTEX);

	sem_control = sem_open(SEM_CONN_CONTROL, O_CREAT, 0660, 0);

	if(sem_control == SEM_FAILED){
		perror("sem_open/conn_control");
		exit(EXIT_FAILURE);
	}

	sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0660, 1);

	if(sem_mutex == SEM_FAILED){

		perror("/mutex");
		exit(EXIT_FAILURE);
	}
 

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

void forward_message(int connfd_sus)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio_src;
	rio_t rio_dst;
	char buf_suscription[MAXLINE];

	sem_wait(sem_control);
	
	int publisher = conn_lst[0];
	int suscriber = conn_lst[1];

	Rio_readinitb(&rio_src, publisher);
	Rio_readinitb(&rio_dst, suscriber);

	Rio_readlineb(&rio_dst, buf_suscription, MAXLINE);
	printf("%s\n", buf_suscription);
	

	if(strcmp(buf_suscription,"node1/cpu_usage\n")==0) {
		while((n = Rio_readlineb(&rio_src, buf, MAXLINE))) {
			//char *buf_cpy[MAXLINE];
			char *pub_available_info[3];

			//memcpy(&buf_cpy, buf, MAXLINE);

			split(buf, ",", pub_available_info);
			printf("Se re-envia: [len: %ld,info: %s]\n", strlen(pub_available_info[1]), pub_available_info[1]);
			strcat(pub_available_info[1], "\n");
			Rio_writen(suscriber, pub_available_info[1], strlen(pub_available_info[1]));

			//close(suscriber);
			//break;
		}
	} 
	

	
}


void *thread(void *vargp)
{
	int connfd = *((int *) vargp);

	sem_wait(sem_mutex);

	conn_lst[conn_ind++] = connfd;  //se agrega sockets de reevio
	if(conn_ind == 2){
		sem_post(sem_control);
	}

	sem_post(sem_mutex);

	printf("Socket: %d\n",connfd);

	pthread_detach(pthread_self());
	free(vargp);
	forward_message(connfd);
	close(connfd);
	return NULL;
}

void split(char *linea, char *delim, char *argv[])  
{
    char *token;
    int i = 0;

    token = strtok(linea, delim);

    while (token != NULL)
    {
        argv[i] = token;
        i++;
        token = strtok(NULL, delim);
    }
}
