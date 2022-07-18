#include "csapp.h"
#include <unistd.h>

#define LIMIT_READ 363 //nos favorece leer hasta esta parte del archivo top.txt


char *host = "127.0.0.1";
char *port = "8090";
char *info_list[MAXLINE]; //contiene la informacion enlistada (se hizo el "split")
char *full_info; //contendra la informacion que se enviara

void update_top_file();
void separar_tokens(char *linea, char *delim, char *argv[]);
void join_full_info(char *cpu, char *tasks, char *memory);
void parse_info(char *memory_percentage);
void change_comma_for_period(char *str);
void update_send_info();

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
		update_top_file(); //se actualiza el archivo top.txt
		update_send_info();
		Rio_writen(clientfd, full_info, strlen(full_info));
		printf("Se ha enviado la informacion de estado...\n");
	}

	Close(clientfd);
	exit(0);
}

void update_top_file(){
	pid_t pid = fork();
	char *args[] = {"", NULL};
	if(pid == 0){
		execv("./exec_top", args);
	}
}

void update_send_info(){
	char read_buffer[LIMIT_READ];
	char cmemory_percentage[10]; //se guardara el porcentaje usado de memoria
	int fd_info_file, read_number; 

	fd_info_file = open("top.txt", O_RDONLY, S_IRUSR | S_IWUSR);
    read_number = read(fd_info_file, read_buffer, LIMIT_READ);
	read_buffer[read_number] = '\0';

	separar_tokens(read_buffer, " ", info_list);

	
    parse_info(cmemory_percentage);
    join_full_info(info_list[24], info_list[14], cmemory_percentage);
	//printf("FULL INFO -%s- len(%ld)\n", full_info, strlen(full_info));
}

void join_full_info(char *cpu, char *tasks, char *memory){
    full_info = (char *) malloc( strlen(cpu) + strlen(tasks) + strlen(memory) + 1);
    strcat(full_info, cpu);
    strcat(full_info, ",");
    strcat(full_info, tasks);
    strcat(full_info, ",");
    strcat(full_info, memory);
    full_info[strlen(full_info) - 1] = '\n';
}

void parse_info(char *memory_percentage)
{
    change_comma_for_period(info_list[14]);
    change_comma_for_period(info_list[24]);

    float used_memory = strtol(info_list[48], NULL, 10);
    float total_memory = strtol(info_list[42], NULL, 10);
    float fmemory_percentage = (used_memory/total_memory)*100;
	
    snprintf(memory_percentage, 10, "%0.4f", fmemory_percentage);
    //printf("char info -%s-\n", memory_percentage);
}

void separar_tokens(char *linea, char *delim, char *argv[])  
{
    char *token;
    int i = 0;

    /* obtencion del primer token */
    token = strtok(linea, delim);

    /* recorre todos los tokens */
    while (token != NULL)
    {
        argv[i] = token;
        i++;
        token = strtok(NULL, delim);
    }

    argv[i] = NULL;

}

void change_comma_for_period(char *str){
    for(int i = 0; i<strlen(str); i++){
        if(str[i] == ','){
            str[i] = '.';
        }
    }
}
