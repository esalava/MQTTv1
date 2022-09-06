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
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define SEM_CONN_CONTROL "/conn_control"
#define SEM_MUTEX "/mutex"
#define SHARINGTIME 1         //Cantidad de veces que reenvia un mensaje a un cliente (1: reenviara 1 mensaje a cada suscriber iterando)
#define LIMIT_CONN_CLIENTS 10
#define TOPICS_NUMBER 6
#define LOG_MSG_SIZE 4096


void forward_message(int connfd_pub, int connfd_sus, int topic[]);
void *thread(void *vargp);
void split(char *linea, char *delim, char *argv[]);
int find_index_by_client_name(char *client_name);
void forward_messages();
int find_topic_by_name(char *topic);
void update_sus_info(char *suscription[10], int information[6]);
void write_log(char *msg);


char *client_name_list[LIMIT_CONN_CLIENTS];//lista que contiene los nombres de los nodos conectados
int client_fd_list[LIMIT_CONN_CLIENTS];    //lista paralela con los descriptores que se encuentran conectados
int fd_idx = 0;							   //lleva la cuenta de cuantos se encuentran conectados 
char *client_sus_list[LIMIT_CONN_CLIENTS]; //Lista de los nodos a los que se encuentran subscrito
int topic_list[LIMIT_CONN_CLIENTS][TOPICS_NUMBER];        //lista de los topicos suscritos

char *string_topics[6] = {"tasks/gen: ", "tasks/run: ", "tasks/sleep: ", "cpu/gen: ", "cpu/sys: ", "mem/gen: "};

pthread_t thread_id;

sem_t *sem_control;
sem_t *sem_mutex;
int fd_log;

void sig_handler(int signum){
	for(int i = 0; i < fd_idx; i++){
		close(client_fd_list[i]);
		client_fd_list[i] = 0;
	}
	fd_idx = 0;
	exit(0);
}

int main(int argc, char **argv)
{	
	signal(SIGINT, sig_handler);
	remove("log_broker.txt");

	fd_log = open("log_broker.txt",  O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
    dup2(fd_log, 1);
    dup2(fd_log, 2); 


	
	
	//Sockets
	int listenfd, *connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	char *port;
	char main_log_message[LOG_MSG_SIZE];
	
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
	//printf("Esperando por conexiones...\n");
	sprintf(main_log_message, "Esperando por conexiones");
	write_log(main_log_message);

	listenfd = Open_listenfd(port);

	//printf("server escuchando en puerto %s...\n", port);
	sprintf(main_log_message, "Servidor escuchando en el puerto: %s", port);
	write_log(main_log_message);

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
void forward_message(int connfd_pub, int connfd_sus, int topic[])
{
	size_t n;
	char buf[MAXLINE];
	rio_t rio_src;
	rio_t rio_dst;
	char main_log_message[MAXLINE*2];
	

	int publisher = connfd_pub;
	int suscriber = connfd_sus;

	Rio_readinitb(&rio_src, publisher);
	Rio_readinitb(&rio_dst, suscriber);

	for(int i = 0; i < SHARINGTIME; i++){
		if((n = Rio_readlineb(&rio_src, buf, MAXLINE))){	

			char tmp_buf[100];
			strcpy(tmp_buf, buf);
			tmp_buf[strlen(tmp_buf)-1] = '\0';
			sprintf(main_log_message, "Se ha recibido [%s] de [%d]", tmp_buf, publisher);
			write_log(main_log_message);

			char *pub_available_info[10];  //informacion disponible para enviar
			split(buf, ",", pub_available_info);  //la informacion de llega separadas por "," se separa
			


			for(int j = 0; j < 6; j++){
				if(topic[j] != -1){ //si es un topico valido a enviar

					char string[50];
					strcpy(string, string_topics[j]);
					strcat(string, pub_available_info[j]);
					string[strlen(string)+1] = '\n';

					if(j < 5){
						strcat(string, "\n");
					}	
					
					
					

					Rio_writen(suscriber, string, strlen(string));
				}
			}

		} else {

			//printf("No hay mensaje disponible para reenviar...\n");
			sprintf(main_log_message, "No hay mensaje disponible para enviar...\n");
					write_log(main_log_message);

		}
	}
}


void *thread(void *vargp)
{
	char main_log_message[MAXLINE*3];
	int connfd = *((int *) vargp);
	pthread_detach(pthread_self());
	//printf("New socket: %d\n",connfd);

	sprintf(main_log_message, "Se ha abierto un socket: %d", connfd);
	write_log(main_log_message);

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
	//printf("Se ha conectado %d", buf_node_name)
	sprintf(main_log_message, "Se ha conectado %s", buf_node_name);
	write_log(main_log_message);

	buf_sus_info[strlen(buf_sus_info)-1] = '\0';   //se obtiene la informacion de la suscripcion del nodo "node1/cpu"

	 
	
	if(strcmp(buf_sus_info, "NONE") == 0 ){  
		//en caso de que no tenga ninguna suscripcion no se guarda ningun topico (su metrica)
		//printf("No tiene ninguna suscripcion\n");
		sprintf(main_log_message, "%s no tiene ninguna suscripcion", buf_node_name);
		write_log(main_log_message);

		client_sus_list[fd_idx] = buf_sus_info;

	} else {
		//se guardan las metricas requeridas en una lista (buf_sus_info)
		split(buf_sus_info, "/", buf_suscription_info);


		client_sus_list[fd_idx] = buf_suscription_info[0]; //se guarda el nombre del nodo que se encuentra subscrito
		int subscribed_info[6]; //= {0,0,0,0,0,0};
		sprintf(main_log_message, "%s Se encuentra suscrito a: %s", buf_node_name, buf_sus_info);
		write_log(main_log_message);

		//se actualiza la informacion dependiendo a que topicos se encuentra suscritos
		update_sus_info(buf_suscription_info, subscribed_info);

		//se copia en la matriz de los topicos suscritos por cada cliente
		for(int k = 0; k < 6; k++){
			topic_list[fd_idx][k] = subscribed_info[k];
		}

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
	int topics_number[6];

	sem_wait(sem_control);
	while(1){  //permite el constante envio de mensajes a los suscriptores
    	for(int i = 0; client_sus_list[i] != NULL; i++){ //recorre la lista de los nodos que tienen una suscripcion
        	if(strcmp(client_sus_list[i], "NONE") != 0){
            	publisher = client_sus_list[i];
            	suscriber = client_name_list[i];

				for(int j = 0; j < 6; j++){
					topics_number[j] = topic_list[i][j];
				}
				

				//informacion de control
            	//printf("Publisher:{%s}, Suscriber:{%s}, Topic {%d}\n", publisher, suscriber, topic_number);

				fd_pub = client_fd_list[find_index_by_client_name(publisher)];
				fd_sus = client_fd_list[find_index_by_client_name(suscriber)];
			
				//informacion de control
				//printf("Intento de envio desde fd_pub:[%d] a fd_sus:[%d]\n", fd_pub, fd_sus);
				char main_log_message[MAXLINE];
				sprintf(main_log_message, "Se envia informacion desde [%s] -> [%s]", publisher, suscriber);
				write_log(main_log_message);

				forward_message(fd_pub, fd_sus, topics_number);  //reenvio del mensaje
        	}
   	 	}	
	}
}


void update_sus_info(char *suscription[10], int information[6]){

		while(1){
        if(strcmp(suscription[1], "#") == 0){
            break;
        } else if (strcmp(suscription[1], "+") == 0) {
            
            if(strcmp(suscription[2], "gen") == 0){

                information[1] = -1;
                information[2] = -1;
                information[4] = -1;

                break;
            } 
            break; 
            
            //no hay ninguna otra información de un nivel
        
        } else {
            if(strcmp(suscription[1],"tasks") == 0 ){

                if(strcmp(suscription[2], "#") == 0){

                    information[3] = -1;
                    information[4] = -1;
                    information[5] = -1;

                    break;

                } else if (strcmp(suscription[2], "gen") == 0){

                    for(int i = 0; i < 6; i++){
                        if(i != 0){
                            information[i] = -1;
                        }
                    }

                    break;
                } else if (strcmp(suscription[2], "run") == 0){

                    for(int i = 0; i < 6; i++){
                        if(i != 1){
                            information[i] = -1;
                        }
                    }

                     break;

                } else {// (strcmp(suscription[1], "sleep") == 0){

                    for(int i = 0; i < 6; i++){
                        if(i != 2){
                            information[i] = -1;
                        }
                    }

                     break;
                }

            } else if(strcmp(suscription[1],"cpu") == 0){

                if(strcmp(suscription[2], "#") == 0){
                    
                    information[0] = -1;
                    information[1] = -1;
                    information[2] = -1;
                    information[5] = -1;
                    
                    break;

                } else if (strcmp(suscription[2], "gen") == 0){

                    for(int i = 0; i < 6; i++){
                        if(i != 3){
                            information[i] = -1;
                        }
                    }

                     break;

                } else { //(strcmp(suscription[1], "sys") == 0){

                    for(int i = 0; i < 6; i++){
                        if(i != 4){
                            information[i] = -1;
                        }
                    }

                     break;
                }

            }

            else /*(strcmp(suscription[0],"ram") == 0)*/{
                
                if(strcmp(suscription[2], "#") == 0){
                    
                    information[0] = -1;
                    information[1] = -1;
                    information[2] = -1;
                    information[3] = -1;
                    information[4] = -1;
                    
                    break;

                } else  /*(strcmp(suscription[1], "gen") == 0)*/{

                    for(int i = 0; i < 6; i++){
                        if(i != 5){
                            information[i] = -1;
                        }
                    }

                     break;
                }

            }

        }
	}
}


void write_log(char *msg){
    time_t t;
    struct tm *tm;
    char message[MAXLINE];

    t=time(NULL);
    tm = localtime(&t);
    strftime(message, MAXLINE, "%d/%m/%Y [%H:%M:%S]: ", tm);
    
    strcat(message, msg);
    printf("%s\n", message);
}
