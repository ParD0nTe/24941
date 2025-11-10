#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For fork(), execlp()
#include <sys/wait.h> // For wait()
int main(void){
    pid_t cpid;
    printf("Forking...\n");
    cpid = fork();
    if(cpid==0){
        execlp("cat","cat","test.txt",NULL);
        exit(0);
    }
    waitpid(cpid,NULL,0);
    printf("\nFinished\n");
    return 0;
}
