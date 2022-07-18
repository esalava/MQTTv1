#include "csapp.h"
#include "MQTTconfig.h"
#include <unistd.h>

#define LIMIT_READ 363 //nos favorece leer hasta esta parte del archivo top.txt

//posiciones en donde se encuentra la informacion del archivo
#define TASKS_POS 17      //13 //14
#define CPU_POS 1         //23 //24
#define MEM_AVAIL_POS 29  //41 //42
#define MEM_USED_POS  35  //47 //48


char *host = BROKER_HOST;   //servidor al que se encuentra conectado
char *port = BROKER_PORT;		//puerto al que se encuentra conectado
char *info_list[MAXLINE];   //contiene la informacion enlistada (se hizo el "split")
char *full_info;            //contendra la informacion que se enviara


void update_top_file();
void filter_top_file();
void separar_tokens(char *linea, char *delim, char *argv[]);
void join_full_info(char *cpu, char *tasks, char *memory);
void parse_info(char *memory_percentage);
void change_comma_for_period(char *str);
void update_send_info();

int main(int argc, char **argv)
{
	int clientfd;
	char *node_name;
	char node_name_buf[20];
	char buf_suscription[] = "NONE\n";
	rio_t rio;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <node_name>\n", argv[0]);
		exit(0);
	}
	
	node_name = argv[1];
	strcpy(node_name_buf, node_name);  //parsing del nombre del nodo
	strcat(node_name_buf, "\n");  //conveniencia


	clientfd = Open_clientfd(host, port);


	Rio_readinitb(&rio, clientfd);
	Rio_writen(clientfd, node_name_buf, strlen(node_name_buf));      //se envia el nombre del nodo al broker
	Rio_writen(clientfd, buf_suscription, strlen(buf_suscription));  //se envia las suscripciones posee al broker (NONE by default)

	printf("Name: %sSuscription: %sConnected to: %s\n\n\n", node_name_buf, buf_suscription,host);

	while (1) {
		sleep(2);
		update_top_file(); //se actualiza el archivo top.txt
		filter_top_file(); //se filtra el archivo top.txt
		update_send_info(); //se actualiza la informacion que se enviara
		Rio_writen(clientfd, full_info, strlen(full_info));
		printf("Se ha enviado la informacion: %s", full_info);
		free(full_info);
	}

	Close(clientfd);
	exit(0);
}

//se actualiza la informacion del archivo top.txt (por medio del programa pivote)
void update_top_file(){
	pid_t pid = fork();
	int wstatus;

	char *args[] = {"", NULL};
	if(pid == 0){
		execv("./exec_top", args);
	}
	waitpid(pid, &wstatus, 0);
}

//se filtra la informacion del archivo top.txt y se almacena en info.txt
void filter_top_file(){
	pid_t pid = fork();
	int wstatus;

	char *args[] = {"", NULL};
	if(pid == 0){
		execv("./exec_grep", args);  //ejecucion de programa pivote
	}
	waitpid(pid, &wstatus, 0);
}

//se actualiza la informacion enviada
void update_send_info(){
	char read_buffer[LIMIT_READ];
	char cmemory_percentage[10]; //se guardara el porcentaje usado de memoria
	int fd_info_file, read_number; 

	fd_info_file = open("info.txt", O_RDONLY, S_IRUSR | S_IWUSR);
    read_number = read(fd_info_file, read_buffer, LIMIT_READ);
	read_buffer[read_number] = '\0';

	separar_tokens(read_buffer, " ", info_list);
	

    parse_info(cmemory_percentage);
    join_full_info(info_list[CPU_POS], info_list[TASKS_POS], cmemory_percentage);

	close(fd_info_file);
}

//se hace un join por el caracter "," de la informacion a enviar (por conveniencia)
void join_full_info(char *cpu, char *tasks, char *memory){
    full_info = (char *) malloc( strlen(cpu) + strlen(tasks) + strlen(memory) + 1);
    strcat(full_info, cpu);
    strcat(full_info, ",");
    strcat(full_info, tasks);
    strcat(full_info, ",");
    strcat(full_info, memory);
    full_info[strlen(full_info) - 1] = '\n';
}

//se realiza un parsing de le informacion y se obtiene el % de uso de la memoria principal
void parse_info(char *memory_percentage)
{
    change_comma_for_period(info_list[TASKS_POS]);
    change_comma_for_period(info_list[CPU_POS]);

    float used_memory = strtol(info_list[MEM_USED_POS], NULL, 10);
    float total_memory = strtol(info_list[MEM_AVAIL_POS], NULL, 10);
    float fmemory_percentage = (used_memory/total_memory)*100;
	
    snprintf(memory_percentage, 10, "%0.4f", fmemory_percentage);
    //printf("char info -%s-\n", memory_percentage);
}

void separar_tokens(char *linea, char *delim, char *argv[])  
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

    argv[i] = NULL;

}

//parsing de la informacion enviada (se cambia las "," por ".")
void change_comma_for_period(char *str){
    for(int i = 0; i<strlen(str); i++){
        if(str[i] == ','){
            str[i] = '.';
        }
    }
}
