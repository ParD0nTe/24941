#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){

    if (argc < 2){
        perror("argc");
        return 1;
    }
    int status;

    pid_t descendant_PID;

    switch(descendant_PID=fork()) {
    case -1:
        perror("fork"); /* произошла ошибка */
        exit(1); /*выход из родительского процесса*/
    case 0:
        execvp(argv[1], &argv[1]);
        perror("execvp");
        return 1;
    default:
        
        wait(&status);
        printf("\nstatus = %d\n", status);
    }

    return 0;
}
