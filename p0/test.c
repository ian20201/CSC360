#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#define OUTPUT printf("%d\n", i)

int main() {
    int i=0;
    OUTPUT;
    if (fork()) {
        waitpid(0,0,0);
        i+=2;
        OUTPUT;
    } else {
        i+=1;
        OUTPUT;
        return(0);
    }
}