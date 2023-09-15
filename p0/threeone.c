//Name : Ian Chen
//V-Number : V00887293
//Date : 2023-09-14
//CSC 360 Assignment 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_IN_COMMAND 100
#define MAX_IN_CHARS 200

void getinput(char* input);
int use_fork(char split[MAX_IN_COMMAND][MAX_IN_CHARS],int args);

void getinput(char* input){
    char *tmpinput = malloc(sizeof(input)+1);
    strcpy(tmpinput,input);

    char split[MAX_IN_COMMAND][MAX_IN_CHARS];
    char *token = strtok(tmpinput," ");
    strcpy(split[0],token);
    int counter = 1;
    if (strcmp(split[0],"")){    
        
        while(token != NULL){

            token = strtok(NULL, " ");
            if(token == NULL)
                break;
            strcpy(split[counter],token);

            counter++;
        }
        // Run all the command from the Input line
    }   

    free(tmpinput);

    if(!strcmp(input,"getcwd()")){
        char directory[200];
        printf("%s\n",getcwd(directory,sizeof(directory)));
        //Get working directory
    }else if(!strcmp(input,"getlogin()")){
        printf("%s\n",getlogin());
        //Get username
    }else if(!strcmp(input,"gethostname()")){
        char hostname[200];
        gethostname(hostname,sizeof(hostname)); //return the host name to b, but the function is int function
        printf("%s\n",hostname);
    }else if(!strcmp(input,"exit")){
        exit(0);
    }else{
        use_fork(split,counter);
    }
}

int use_fork(char split[MAX_IN_COMMAND][MAX_IN_CHARS],int args){
    pid_t pid;
    char* command = split[0];
    char* argument_list[args+1];

    for(int counter = 0; counter < args; counter++){
        argument_list[counter] = malloc(sizeof(split[counter+1])+1);
        strcpy(argument_list[counter],split[counter]);
        //Save all the command into the argument_list
    }
    argument_list[args] = NULL;

    // pid_t pid;
    // time_t t;
    // int status;

    // if ((pid = fork()) < 0)
    //     perror("fork() error");
    // else if (pid == 0) {
    //     int status_code = execvp(command, argument_list);
    //     exit(1);
    // }
    // else do {
    //     if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
    //     perror("wait() error");
    //     else if (pid == 0) {
    //     // time(&t);
    //     // printf("child is still running at %s", ctime(&t));
    //     // sleep(1);
    //     }
    //     else {
    //     if (WIFEXITED(status))
    //         printf("child exited with status of %d\n", WEXITSTATUS(status));
    //     else puts("child did not exit successfully");
    //     }
    // } while (pid == 0);
    

    int status;
    pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:{
            // puts("Child exiting.");
            // exit(EXIT_SUCCESS);
            // Newly spawned child Process. This will be taken over by "ls -l"
            int status_code = execvp(command, argument_list);
            // printf("ls -l has taken control of this child process. This won't execute unless it terminates abnormally!\n");
            exit(1);
            if (status_code == -1) {
                printf("Terminated Incorrectly\n");
                return 1;
            }
        }
    default:
        do {
            if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
                perror("wait() error");
            else if (pid == 0) {
                // time(&t);
                // printf("child is still running at %s", ctime(&t));
                // sleep(1);
            }
            else {
                if (WIFEXITED(status));
                    // printf("child exited with status of %d\n", WEXITSTATUS(status));
                else puts("child did not exit successfully");
            }
        } while (pid == 0);        
    }            
    for(int counter = 0; counter < args; counter++)
        free(argument_list[counter]);
    //Reference https://man7.org/linux/man-pages/man2/fork.2.html
    //Reference https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus
    //Reference https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-waitpid-wait-specific-child-process-end
}

int main(int argc, char*argv[]){

    char* input;
    char* prompt = {};

    while ((input = readline("> ")) != NULL) {
        if (strlen(input) > 0) {
        add_history(input);
        }
        getinput(input);
        // readline malloc's a new buffer every time.
        free(input);
    }

    // Reference https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/



}