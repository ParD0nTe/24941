#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc,char **argv){
    if(argc==1){
        return 0;
    }
    pid_t cpid;
    printf("Forking...\n");
    cpid = fork();
    if(cpid==0){
        execvp(argv[1],&argv[1]);
        exit(0);
    }
    int stat_lock;
    waitpid(cpid,&stat_lock,0);
    printf("Finished with code %d\n",stat_lock);
    return 0;
}
