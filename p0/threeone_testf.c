#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void printPrompt();
void addProcessToList(pid_t pid, char* command);
void addTerminatedProcessToList(pid_t pid, char* command);
void removeProcessFromList(pid_t pid);
void printBackgroundProcesses();
void checkTerminatedProcesses();
void sigchld_handler(int signo);
void printTerminatedProcesses();
void executeBackgroundCommand(char* command, char** args);
void handleUserInput(char* input);

typedef struct BackgroundProcess {
    pid_t pid;
    char* command;
    struct BackgroundProcess* next;
} BackgroundProcess;

BackgroundProcess* head = NULL;
int newline_needed = 0;
int prompt_printed = 0;

typedef struct TerminatedProcess {
    pid_t pid;
    char* command;
    struct TerminatedProcess* next;
} TerminatedProcess;

TerminatedProcess* terminated_head = NULL;

BackgroundProcess* createNode(pid_t pid, char* command) {
    BackgroundProcess* newNode = (BackgroundProcess*)malloc(sizeof(BackgroundProcess));
    newNode->pid = pid;
    newNode->command = strdup(command);
    newNode->next = NULL;
    return newNode;
}

TerminatedProcess* createTerminatedNode(pid_t pid, char* command) {
    TerminatedProcess* newNode = (TerminatedProcess*)malloc(sizeof(TerminatedProcess));
    newNode->pid = pid;
    newNode->command = strdup(command);
    newNode->next = NULL;
    return newNode;
}

void addProcessToList(pid_t pid, char* command) {
    BackgroundProcess* newNode = createNode(pid, command);
    if (!head) {
        head = newNode;
    } else {
        BackgroundProcess* current = head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
    printf("Added background process: PID: %d, Command: %s\n", pid, command);
}

void addTerminatedProcessToList(pid_t pid, char* command) {
    TerminatedProcess* newNode = createTerminatedNode(pid, command);
    if (!terminated_head) {
        terminated_head = newNode;
    } else {
        TerminatedProcess* current = terminated_head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void removeProcessFromList(pid_t pid) {
    if (!head) return;
    if (head->pid == pid) {
        BackgroundProcess* temp = head;
        head = head->next;
        free(temp->command);
        free(temp);
        return;
    }
    BackgroundProcess* current = head;
    while (current->next && current->next->pid != pid) {
        current = current->next;
    }
    if (current->next) {
        BackgroundProcess* temp = current->next;
        current->next = current->next->next;
        free(temp->command);
        free(temp);
    }
}

void printBackgroundProcesses() {
    BackgroundProcess* current = head;
    int count = 0;
    printf("\nBackground processes:\n");
    while (current) {
        printf("%d: %s\n", current->pid, current->command);
        current = current->next;
        count++;
    }
    printf("Total Background jobs: %d\n\n", count);
}

void checkTerminatedProcesses() {
    BackgroundProcess* current = head;
    while (current) {
        if (waitpid(current->pid, NULL, WNOHANG) > 0) {
            addTerminatedProcessToList(current->pid, current->command);
            BackgroundProcess* temp = current;
            current = current->next;
            removeProcessFromList(temp->pid);
        } else {
            current = current->next;
        }
    }
}

void sigchld_handler(int signo) {
    (void) signo;
    newline_needed = 1;
    checkTerminatedProcesses();
    printTerminatedProcesses();
    printPrompt();
    prompt_printed = 1;
}

void printTerminatedProcesses() {
    while (terminated_head) {
        printf("\n%d: %s has terminated.\n", terminated_head->pid, terminated_head->command);
        TerminatedProcess* temp = terminated_head;
        terminated_head = terminated_head->next;
        free(temp->command);
        free(temp);
    }
    if (newline_needed) {
        printf("\n");
        newline_needed = 0;
    }
}

void executeBackgroundCommand(char* command, char** args) {
    pid_t pid = fork();
    if (pid == 0) {
        setpgid(0, 0);
        execvp(command, args);
        perror("execvp");
        exit(1);
    } else {
        setpgid(pid, pid);
        char fullCommand[100] = {0};
        for (int i = 0; args[i]; i++) {
            strcat(fullCommand, args[i]);
            strcat(fullCommand, " ");
        }
        addProcessToList(pid, fullCommand);
    }
}

void handleUserInput(char* input) {
    if (strncmp(input, "bg ", 3) == 0) {
        char* command = strtok(input + 3, " ");
        char* args[10] = {0};
        int i = 0;
        args[i++] = command;
        char* token;
        while ((token = strtok(NULL, " "))) {
            args[i++] = token;
        }
        args[i] = NULL;
        executeBackgroundCommand(command, args);
    } else if (strcmp(input, "bglist") == 0) {
        printBackgroundProcesses();
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            char* args[10] = {0};
            char* token;
            int i = 0;
            token = strtok(input, " ");
            while (token) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void printPrompt() {
    char hostname[100];
    char cwd[200];
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    printf("(null)@%s: %s > ", hostname, cwd);
    fflush(stdout);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    char input[100];
    while (1) {
        if (!prompt_printed) {
            printPrompt();
        } else {
            prompt_printed = 0;
        }
        
        fgets(input, sizeof(input), stdin);
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        handleUserInput(input);
    }
    return 0;
}
