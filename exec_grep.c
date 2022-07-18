/* 
    Programa encargado de filtrar la informacion por el uso del grep

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h>


#define MAXLINE 20

void open_program(char  ** arglist);
void separar_tokens(char *linea, char *delim, char *argv[]);

int main(int argc, char *argv[]){
    char cpu_command_buf[MAXLINE] = "grep Cpu top.txt";
    char tasks_command_buf[MAXLINE] = "grep Tasks top.txt";
    char mem_command_buf[MAXLINE] = "grep Mem top.txt";
    
    char delim[2] = " "; /* character delimitation */
    char *arglist[MAXLINE];
    
    separar_tokens(cpu_command_buf, delim, arglist);
    open_program(arglist);

    separar_tokens(tasks_command_buf, delim, arglist);
    open_program(arglist);
    
    separar_tokens(mem_command_buf, delim, arglist);
    open_program(arglist);
    
    return 0;
}

void open_program(char  ** arglist)
{
    int fd = open("info.txt", O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);

    dup2(fd, 1); //redirige stdout al fd
    dup2(fd, 2); //redirige stderr al fd

    close(fd);
    pid_t pid;
    int wstatus;

    if((pid = fork()) == 0){
        execvp(arglist[0], arglist);
    }

    waitpid(pid, &wstatus, 0);

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