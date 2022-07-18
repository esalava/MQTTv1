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
#include "MQTTconfig.h"

#define SEM_CONN_CONTROL "/conn_control"
#define SEM_MUTEX "/mutex"
#define SHARINGTIME 1         //Cantidad de veces que reenvia un mensaje a un cliente (1: reenviara 1 mensaje a cada suscriber iterando)
#define LIMIT_CONN_CLIENTS 10

void forward_message(int connfd_pub, int connfd_sus, int topic);
void *thread(void *vargp);
void split(char *linea, char *delim, char *argv[]);
int find_index_by_client_name(char *client_name);
void forward_messages();
int find_topic_by_name(char *topic);


char *client_name_list[LIMIT_CONN_CLIENTS];//lista que contiene los nombres de los nodos conectados
int client_fd_list[LIMIT_CONN_CLIENTS];    //lista paralela con los descriptores que se encuentran conectados
int fd_idx = 0;							   //lleva la cuenta de cuantos se encuentran conectados 
char *client_sus_list[LIMIT_CONN_CLIENTS]; //Lista de los nodos a los que se encuentran subscrito
int topic_list[LIMIT_CONN_CLIENTS];        //posible valores 0 = CPU, 1 = TASKS, 2 = RAM


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

	port = BROKER_PORT;

	printf("Esperando por conexiones...\n");
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

	sem_close(sem_control);
	sem_close(sem_mutex);

}


//reenvio de mensaje de un publisher determinado a un suscriptor determinado por un topico
void forward_message(int connfd_pub, int connfd_sus, int topic)
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio_src;
	rio_t rio_dst;

	int publisher = connfd_pub;
	int suscriber = connfd_sus;

	Rio_readinitb(&rio_src, publisher);
	Rio_readinitb(&rio_dst, suscriber);

	for(int i = 0; i < SHARINGTIME; i++){
		if((n = Rio_readlineb(&rio_src, buf, MAXLINE)) && topic != -1){	

			char *pub_available_info[3];
			split(buf, ",", pub_available_info);  //la informacion de llega separadas por "," se separa

			printf("Se re-envia: [len: %ld,info: %s] desde fd_src:[%d] -> fd_dst:[%d]\n", 
				strlen(pub_available_info[1]), 
				pub_available_info[topic],
				publisher,
				suscriber);

			//no se debe agregar un salto de linea a [2] porque ya tiene
			if(topic < 2){
				strcat(pub_available_info[topic], "\n");
			}
			
			Rio_writen(suscriber, pub_available_info[topic], strlen(pub_available_info[topic]));
			
		} else {

			printf("No hay mensaje disponible para reenviar...\n");

		}
	}
}


void *thread(void *vargp)
{
	int connfd = *((int *) vargp);
	pthread_detach(pthread_self());
	printf("New socket: %d\n",connfd);

	sem_wait(sem_mutex); //seccion critica

	client_fd_list[fd_idx] = connfd;  //se agrega sockets de reevio

	//orden de los descriptores
	char buf_node_name[MAXLINE];
	
	char *buf_suscription_info[2];       //puntero hacia el nombre = [0] y el topico = [1]
	char buf_sus_info[MAXLINE];			 //buffer donde se guarda la informacion

	
	rio_t rio;
	Rio_readinitb(&rio, connfd);
	Rio_readlineb(&rio, buf_node_name, MAXLINE);
	Rio_readlineb(&rio, buf_sus_info, MAXLINE);
	
	//parsing
	buf_node_name[strlen(buf_node_name)-1] = '\0'; //se obtiene el nombre del nodo
	client_name_list[fd_idx] = buf_node_name;

	buf_sus_info[strlen(buf_sus_info)-1] = '\0';   //se obtiene la informacion de la suscripcion del nodo "node1/cpu"

	 
	
	if(strcmp(buf_sus_info, "NONE") == 0 ){  
		//en caso de que no tenga ninguna suscripcion no se guarda ningun topico (su metrica)
		printf("No tiene ninguna suscripcion\n");
		client_sus_list[fd_idx] = buf_sus_info;

	} else {
		//se guardan las metricas requeridas
		split(buf_sus_info, "/", buf_suscription_info);

		printf("[%s] esta suscrito a: [%s] del topico [%s]\n", client_name_list[fd_idx], buf_suscription_info[0], buf_suscription_info[1]);

		client_sus_list[fd_idx] = buf_suscription_info[0]; 					//se guarda el nombre del nodo que se encuentra subscrito
		topic_list[fd_idx] = find_topic_by_name(buf_suscription_info[1]);	//se guarda el nombre del topico al que se encuentra subscrito
	}

	fd_idx += 1;

	if(fd_idx >= 2){ 
		//por lo menos deben existir dos conexiones para poder empezar el reenvio de mensajes
		sem_post(sem_control);
	}

	sem_post(sem_mutex); //fin seccion critica

	free(vargp);
	forward_messages(connfd); 

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

int find_index_by_client_name(char *client_name){
    for(int i = 0; client_name_list[i] != NULL; i++){
        if(strcmp(client_name, client_name_list[i]) == 0){
            return i;
        }   
    }
    return -1;
}


//permite encontrar el indice del topico al que se encuentra subscrito
int find_topic_by_name(char *topic){
    
    if(strcmp(topic, "cpu") == 0){
        return 0;
    } else if (strcmp(topic, "tasks") == 0) {
		return 1;
	} else  if (strcmp(topic, "ram") == 0){
		return 2;
	}
	return -1; //error
}


//reenvio de mensajes a cada uno de los nodos que se encuentran conectados
void forward_messages(){
	char *publisher;
	char *suscriber;
	int fd_pub;
	int fd_sus;
	int topic_number;

	sem_wait(sem_control);
	while(1){  //permite el constante envio de mensajes a los suscriptores
    	for(int i = 0; client_sus_list[i] != NULL; i++){ //recorre la lista de los nodos que tienen una suscripcion
        	if(strcmp(client_sus_list[i], "NONE") != 0){
            	publisher = client_sus_list[i];
            	suscriber = client_name_list[i];
				topic_number = topic_list[i];

				//informacion de control
            	printf("Publisher:{%s}, Suscriber:{%s}, Topic {%d}\n", publisher, suscriber, topic_number);

				fd_pub = client_fd_list[find_index_by_client_name(publisher)];
				fd_sus = client_fd_list[find_index_by_client_name(suscriber)];
			
				//informacion de control
				printf("Intento de envio desde fd_pub:[%d] a fd_sus:[%d]\n", fd_pub, fd_sus);
			
				forward_message(fd_pub, fd_sus, topic_number);  //reenvio del mensaje
        	}
   	 	}	
	}
}
