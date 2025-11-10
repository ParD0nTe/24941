#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For fork(), execlp()
#include <sys/wait.h> // For wait()
#define SUBPROCESS "echo"
#define ARGS SUBPROCESS,"-c",NULL
int main(void){
    pid_t cpid;
    printf("Forking...\n");
    cpid = fork();
    if(cpid==0){
        execlp(SUBPROCESS,ARGS);
        exit(0);
    }
    int stat_lock;
    waitpid(cpid,&stat_lock,0);
    printf("Finished with code %d\n",stat_lock);
    return 0;
}
