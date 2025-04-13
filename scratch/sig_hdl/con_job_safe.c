#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/types/sigset_t.h>
#include <bits/sigaction.h>

#define JOBNUM 10
pid_t jobs[10];
int CNTADD = 0;
int CNTDEL = 0;

void initjobs(void){
    for(int i=0; i<JOBNUM; i++)
    jobs[i] = 0;
}

void addjob(pid_t pid){
    for(int i=0; i<JOBNUM; i++){
        if(jobs[i] == 0)
        jobs[i] = pid;
        CNTADD += 1;
        return;
    }
    printf("job list is full!!!!!!\n");
    exit(1);
}

void deljob(pid_t pid){
    for(int i=0; i<JOBNUM; i++){
        if(jobs[i] == pid)
        jobs[i] = 0;
        CNTDEL += 1;
        return;
    }
    printf("non-exist pid: %d\n", pid);
    exit(1);
}

void handler(int sig){
    int pid;
    sigset_t mask_all, prev;
    printf("a sig %d captured\n", sig);

    while ((pid=wait(NULL))>0){
        printf("a child is reaped\n");
        sigprocmask(SIG_BLOCK, &mask_all, &prev);
        deljob(pid);
        sigprocmask(SIG_SETMASK, &prev, NULL);
        
    }
}


int main() {
    int pid;
    initjobs();
    signal(SIGCHLD, handler);
    sigset_t mask_all, prev, mask_chld;
    
    sigfillset(&mask_all);
    sigemptyset(&mask_chld);
    sigaddset(&mask_chld, SIGCHLD);
    
    for(int i=0; i<100; i++){
        sigprocmask(SIG_BLOCK, &mask_chld, &prev);
        if((pid=fork()) == 0){
            sigprocmask(SIG_SETMASK, &prev, NULL);
            exit(0);
        }
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        addjob(pid);
        sigprocmask(SIG_SETMASK, &prev, NULL);
        
    }
    sleep(2);
    printf("add: %d, del: %d\n", CNTADD, CNTDEL);
    exit(0);
}