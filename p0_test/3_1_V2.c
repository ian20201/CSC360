#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h> // 信号处理库

typedef struct bg_process {
    int pid;
    char command[256];
} bg_process;

bg_process bg_processes[100];
int bg_process_count = 0;

void parse_input(char input[], char* arr[]);
void execute_command(char* arr[]);
void change_dir(char* arr[]);
void background_exe(char* arr[]);
void display_bg_processes();
void check_bg_processes();
void handle_sigchld(int sig);
volatile sig_atomic_t child_terminated = 0;
void display_prompt();

int main(int argc, char *argv[]) {
    // 设置SIGCHLD信号处理器
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
    display_prompt();
    while(1) {  // 使用 while(1) 替代 for(;;) 作为无限循环
        char input[256];
        char* arr[100] = {0};
        // 使用 fgets 而不是 scanf 来读取输入
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;  // 如果发生错误或到
        }

        // 如果存在，删除尾部的换行字符
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        parse_input(input, arr);
        change_dir(arr);
        if(!child_terminated) {
            display_prompt(); // 使用新的函数来显示命令提示符
        }
        child_terminated = 0;
    }
    return 0;
}
void change_dir(char* arr[]){
    
    
    if(arr[0]!=NULL){
        if(strcmp(arr[0],"bglist")==0){
            display_bg_processes();
            return;
        }
        
        
        if(strcmp(arr[0],"bg")==0){
            background_exe(arr);
            return;
        }
        //处理cd
        if(strcmp(arr[0],"cd")==0){
            if(arr[1]==NULL){
                chdir(getenv("HOME"));
                return;
            }
            if(strcmp(arr[1],"~")==0){
                chdir(getenv("HOME"));
                return;
            }
            if(chdir(arr[1])==-1){
                printf("Invalid Directory\n");
                return;
            }     
        }else{
        execute_command(arr);
        }
    }
}
void parse_input(char input[], char* arr[]) {
    int i = 0;
    char* token = strtok(input, " ");
    while(token != NULL) {
        arr[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
}

void execute_command(char* arr[]) {
    int pid = fork();
    if(pid == 0) {
        if(execvp(arr[0], arr) == -1) {
            printf("Invliad Command\n");
            exit(0);
        }
        exit(0);
    } else {
        waitpid(pid, NULL, 0);
    }
}

void background_exe(char* arr[]) {
    int pid = fork();
    if(pid == 0) {
        if(execvp(arr[1], &arr[1]) == -1) {
            perror("Error executing command");
            exit(0);
        }
        exit(0);
    } else {
        bg_processes[bg_process_count].pid = pid;
        strcpy(bg_processes[bg_process_count].command, arr[1]);
        for (int i = 2; arr[i] != NULL; i++) {
            strcat(bg_processes[bg_process_count].command, " ");
            strcat(bg_processes[bg_process_count].command, arr[i]);
        }
        bg_process_count++;
        printf("Added background process: PID: %d, Command: %s\n", pid, bg_processes[bg_process_count].command);
        usleep(100000); // 等待100ms
    }
}

void display_bg_processes() {
    printf("\nBackground processes:\n");
    for (int i = 0; i < bg_process_count; i++) {
        printf("%d: %s\n", bg_processes[i].pid, bg_processes[i].command);
    }
    printf("Total Background jobs: %d\n\n", bg_process_count);
}

void check_bg_processes() {
    int status;

    for (int i = 0; i < bg_process_count; i++) {
        if (waitpid(bg_processes[i].pid, &status, WNOHANG) != 0) {
            printf("\n%d: %s has terminated.\n", bg_processes[i].pid, bg_processes[i].command);
            for (int j = i; j < bg_process_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_process_count--;
            i--;
        }
    }
}

void display_prompt() {
    char cwd[256];
    char hostname[50];
    gethostname(hostname, sizeof(hostname));
    printf("%s@%s: %s > ", getlogin(), hostname, getcwd(cwd, sizeof(cwd)));
    fflush(stdout); // 确保立即输出
}

void handle_sigchld(int sig) {
    (void)sig; // 避免出现未使用的参数
    check_bg_processes();
    display_prompt();
    child_terminated = 1;
}