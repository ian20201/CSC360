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

void print_prompt();
void get_directory();
void get_hostname();
void getinput(char* input);
int use_fork(char split[MAX_IN_COMMAND][MAX_IN_CHARS],int args);
int change_directory(char* argument_list[MAX_IN_COMMAND],int args);

int main(int argc, char*argv[]){
    char* input = NULL;    
    while (1) {
        print_prompt();
        input = readline("");
        //readline malloc's a new buffer every time.
        if (strlen(input) > 0) {
            add_history(input);
        }
        getinput(input);

        free(input);
    }
    // Reference https://eli.thegreenplace.net/2016/basics-of-using-the-readline-library/
    return 0;
}

void print_prompt(){
    char directory_main[200];
    char hostname_main[200];
    char* prompt[] = {getlogin(),"@",hostname_main,": ",directory_main," >"};
    //getloin() is use to get the Username
    get_directory(directory_main);
    get_hostname(hostname_main);
    //Reset the directoy forevery execution
    printf("%s%s%s%s%s%s ",prompt[0],prompt[1],prompt[2],prompt[3],prompt[4],prompt[5]);
    //Print the current working directory
}

void get_directory(char *directory){
    getcwd(directory,sizeof(char)*200);
    //Get working directory
}

void get_hostname(char *hostname){
    gethostname(hostname,MAX_IN_CHARS); 
    //Return the host name
}

void getinput(char* input){
    char *tmpinput = malloc(sizeof(input)+1);
    strcpy(tmpinput,input);

    char split[MAX_IN_COMMAND][MAX_IN_CHARS];
    char *token = strtok(tmpinput," ");

    if(token != NULL){
    //Check if the token is NULL or not for the ENTER KEY check!!
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
            // split all the command from the Input line to the array
        }   

        free(tmpinput);

        if(!strcmp(split[0],"exit")){
            printf("Exit the System\n");
            exit(0);
        }else{
            use_fork(split,counter);
        }
    }else{
       printf("\n"); 
    }
}


int use_fork(char split[MAX_IN_COMMAND][MAX_IN_CHARS],int args){
    pid_t pid;
    char* command = split[0];
    char* argument_list[args+1];
    if(!strcmp(command,"bg")){
        for(int counter = 1; counter < args; counter++){
        argument_list[counter] = malloc(sizeof(split[counter+1])+1);
        strcpy(argument_list[counter],split[counter]);
        //Save all the command into the argument_list
    }
    }else{
        for(int counter = 0; counter < args; counter++){
        argument_list[counter] = malloc(sizeof(split[counter+1])+1);
        strcpy(argument_list[counter],split[counter]);
        //Save all the command into the argument_list
    }  
    }
    // for(int counter = 0; counter < args; counter++){
    //     argument_list[counter] = malloc(sizeof(split[counter+1])+1);
    //     strcpy(argument_list[counter],split[counter]);
    //     //Save all the command into the argument_list
    // }
    argument_list[args] = NULL;  

    int status;
    pid = fork();
    printf("PID: %d\n",pid);
    switch (pid) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:{
                int status_code;
                if(!strcmp(command,"cd")){
                    status_code = change_directory(argument_list,args);
                    //Use for the cd command processes
                    // printf("PID: %d\n",pid);    
                }else if(!strcmp(command,"bg")){
                    printf("bg command found\n");
                    // status_code = execvp(command, argument_list);
                    char *test[] = {"cat","test.txt",NULL};
                    status_code = execvp(test[0], test);
                    //Run the bg command in back ground
                    // waitpid(pid, &status, WNOHANG);
                }else{
                    status_code = execvp(command, argument_list);
                    //Run the original command for ls or other command
                }                
                // status_code = execvp(command, argument_list);
                if (status_code == -1) {
                    printf("Terminated Incorrectly\n");
                    return 1;
                }
            }
        default:
            // do {
            //     if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
            //         perror("wait() error");
            //     // else if (pid == 0) {
            //     //     time(&t);
            //     //     printf("child is still running at %s", ctime(&t));
            //     //     sleep(1);
            //     // }
            //     else {
            //         if (WIFEXITED(status));
            //         else puts("child did not exit successfully");
            //     }
            // } while (pid == 0);
                    if(!strcmp(command,"bg")){
                        printf("bg wait\n");
                        // char *test[] = {"cat","test.txt",NULL};
                        // execvp(test[0], test);
                        // waitpid(pid, &status, WNOHANG);
                        //pid_t waitpid(pid_t pid, int *status_ptr, int options); 
                        //WNOHANG mant dont wait for the child process to end
                    }else{
                        printf("PID3: %d\n",pid);
                        waitpid(pid,&status,0);
                        //pid_t waitpid(pid_t pid, int *status_ptr, int options); 
                        //0 here mean to wait for the child process 
                        printf("PID2: %d\n",pid);
                    }

            // waitpid(pid,&status,0);
            //pid_t waitpid(pid_t pid, int *status_ptr, int options); 
            //0 here mean to wait for the child process   
    }            
    for(int counter = 0; counter < args; counter++)
        free(argument_list[counter]);
    //Reference https://man7.org/linux/man-pages/man2/fork.2.html
    //Reference https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus
    //Reference https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-waitpid-wait-specific-child-process-end
}

int change_directory(char* argument_list[MAX_IN_COMMAND],int args){
    int status_code;
    if(args > 2){
        printf("Ivalid cd command\n");
    }else if(argument_list[1] == NULL){
        status_code = chdir(getenv("HOME"));
        //Nessesy to dected the NULL argument before the using the strcmp, dected the cd with nothing at back
    }else if(!strcmp(argument_list[1],"~")){
        status_code = chdir(getenv("HOME"));
        //Change the director to home directory
    }else{
        status_code = chdir(argument_list[1]);
        //Run the regular change directory
    }
    return status_code;
    //Return the status code for the status check
}