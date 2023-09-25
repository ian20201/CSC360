//Name : Ian Chen
//V-Number : V00887293
//Date : 2023-09-14
//CSC 360 Assignment 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h> //Signal control for the background process check
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_IN_COMMAND 100
#define MAX_IN_CHARS 200

typedef struct bg_process{
    int pid;
    char command[MAX_IN_COMMAND];
    struct bg_process* next;   
} bg_process;

bg_process* bg_process_list = NULL;
int bg_process_list_count = 0;
volatile sig_atomic_t child_terminated = 0;

void print_prompt();
void get_directory();
void get_hostname();
void getinput(char* input);
int use_fork(char split[MAX_IN_COMMAND][MAX_IN_CHARS],int args);
int change_directory(char* argument_list[MAX_IN_COMMAND],int args);
void string_casting(char* command[MAX_IN_COMMAND],char* return_command);
bg_process* add_bg_process(int pid, char* command);
void remove_bg_process(bg_process *target_process);
void display_bg_process();
void check_bg_process_status();
void handle_sigchld(int sig);


int main(int argc, char*argv[]){
    char* input = NULL;
    struct sigaction background_check;
    background_check.sa_handler = &handle_sigchld;
    background_check.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    //Reference https://linux.die.net/man/2/sigaction
    if(sigaction(SIGCHLD, &background_check,0) == -1){
        perror("sigaction");
        exit(1);
    }

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
            // Split all the command from the Input line to the array
        }   

        free(tmpinput);

        if(!strcmp(split[0],"exit")){
            printf("Exit the System\n");
            exit(0);
        }else if(!strcmp(split[0],"bglist")){
            display_bg_process();
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
        // printf("bg list \n");
        for(int counter = 1; counter < args; counter++){
        argument_list[counter-1] = malloc(sizeof(split[counter])+1);
        strcpy(argument_list[counter-1],split[counter]);
        //Save all the command into the argument_list for bg    
        }
        argument_list[args-1] = NULL;
    }else{
        for(int counter = 0; counter < args; counter++){
        argument_list[counter] = malloc(sizeof(split[counter+1])+1);
        strcpy(argument_list[counter],split[counter]);
        //Save all the command into the argument_list 
        }
        argument_list[args] = NULL;  
    }
      
    // printf("%s %s\n",argument_list[0],argument_list[1]);

    int status;
    pid = fork();
    // printf("PID: %d\n",pid);
    int status_code;
    switch (pid) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:{
                // int status_code;
                if(!strcmp(command,"cd")){
                    status_code = change_directory(argument_list,args);
                    //Use for the cd command processes
                    // printf("PID: %d\n",pid);    
                }else if(!strcmp(command,"bg")){
                    printf("bg command found\n");
                    // char *test[] = {"cat","test.txt",NULL};
                    // status_code = execvp(test[0], test);
                    status_code = execvp(argument_list[0], argument_list);
                    //Run the bg command in back ground
                    exit(0);
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
                        printf("PID_bg: %d\n",pid);
                        printf("bg wait\n");
                        // waitpid(0, &status, WNOHANG);
                        //pid_t waitpid(pid_t pid, int *status_ptr, int options); 
                        //WNOHANG mant dont wait for the child process to end
                        char *casting_command;
                        casting_command = malloc(sizeof(MAX_IN_CHARS)+1);
                        //Creat the char pointer tp stpre the command
                        string_casting(argument_list,casting_command);
                        //Make all the command to in argument_list become one sentence in casting_command
                        if(status_code != -1)
                            add_bg_process(pid,casting_command);
                        //Add the PID and the command into the struct pointer
                        printf("PID_bg: %d Command: %s\n",pid,casting_command);
                        free(casting_command); // add
                    }else{
                        printf("PID3: %d\n",pid);
                        waitpid(pid,&status,0);
                        //pid_t waitpid(pid_t pid, int *status_ptr, int options); 
                        //0 here mean to wait for the child process 
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

void string_casting(char* command[MAX_IN_COMMAND],char* return_command){
    int counter = 0;
    strcpy(return_command,"");
    //Clear what is in the command befor use
    while (command[counter] != NULL)
    {
        strcat(return_command,command[counter]);
        if(command[counter+1]!=NULL)
            strcat(return_command," ");
        counter++;
    }
    //Make all the command to in argument_list become one sentence in casting_command
}

bg_process* add_bg_process(int pid, char* command){
    bg_process* new_process = malloc(sizeof(bg_process));
    new_process->pid = pid;
    strcpy(new_process->command,command);
    new_process->next = bg_process_list;
    bg_process_list = new_process;
    bg_process_list_count++;
    return new_process;
    //Add the PID and the command into the struct pointer
}

void remove_bg_process(bg_process *target_process){
    if(!target_process) return;
    if(bg_process_list == target_process){
        bg_process_list = target_process->next;
    }else{
        bg_process* tmp = bg_process_list;
        while (tmp->next && tmp->next != target_process)
        {
            tmp = tmp->next;
        }
        if(tmp->next){
            tmp->next = target_process->next;
        }
        free(target_process);
        bg_process_list_count--;
    }
}

void display_bg_process(){
    printf("\nBackgroud Processes: \n");
    for(bg_process* tmp = bg_process_list;tmp; tmp = tmp->next){
        printf("PID %d: Command %s\n",tmp->pid, tmp->command);
    }
    printf("Total Background Jobs: %d\n\n", bg_process_list_count);
}

void check_bg_process_status(){
    int status;
    bg_process* current_p = bg_process_list;
    while (current_p)
    {
        if(waitpid(current_p->pid, &status, WNOHANG) != 0){
            printf("\nPID %d: %s has terminated. \n", current_p->pid, current_p->command);
            bg_process* tmp = current_p;
            current_p = current_p->next;
            remove_bg_process(tmp);
        } else {
            current_p = current_p->next;
        }
    }
    fflush(stdout); // 确保输出被立即打印
    print_prompt();
}

void handle_sigchld(int sig){
    (void)sig;
    check_bg_process_status();
    // int status = 0;
    // status = check_bg_process_status();
    // if(status == 1)
    // print_prompt();
    fflush(stdout); // add
    child_terminated = 1;
    // printf(" ");
}