#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


void handler(int sig){
    printf("a sig is captured by handler: %d\n", sig);
    wait(NULL);
    printf("a child is reaped\n");
}

void handler_loop(int sig){
    printf("a sig is captured by handler: %d\n", sig);
    while(wait(NULL) > 0)
        printf("a child is reaped\n");
}

int main() {
    signal(SIGCHLD, handler_loop);
    int i;
    for (i=0; i<5; i++){
        if (fork() == 0) {
            printf("im a child %d, my parent is %d\n", (int)getpid(), (int)getppid());
            exit(0);
        }
    }
    sleep(2);
    exit(0);
}