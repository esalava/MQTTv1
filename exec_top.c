#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h> 
#include<sys/stat.h>

#define MAXLINE 20

void open_program(char  ** arglist);
void separar_tokens(char *linea, char *delim, char *argv[]);

int main(int argc, char *argv[])
{
    char command_buf[MAXLINE] = "top -b -n 1 -E m";
    char delim[2] = " "; /* character delimitation */
    char *arglist[MAXLINE];
    int fd[2];  //using pipe
    int wstatus;

    //pipe creation
    if(pipe(fd) < 0)
        exit(1);

    separar_tokens(command_buf, delim, arglist);

    pid_t pid = fork();

    if(pid == 0){
        open_program(arglist);
        exit(0);
    }

    while(waitpid(-1, &wstatus, WUNTRACED | WCONTINUED) == -1);
    return 0;
}

void open_program(char  ** arglist)
{
    int fd = open("top.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    dup2(fd, 1); //redirige stdout al fd
    dup2(fd, 2); //redirige stderr al fd

    close(fd);

    if(execvp(arglist[0], arglist) == -1){ /* child executes program */

        //on error
        printf("not a valid command, try again :)\n");   
    }

    //not reachable on success

    exit(0); //exit on evecvp failure

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