#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){

    pid_t descendant_PID;

    switch(descendant_PID=fork()) {
    case -1:
        perror("fork"); /* произошла ошибка */
        exit(1); /*выход из родительского процесса*/
    case 0:
        execlp("cat", "cat", "Text.txt", NULL);
        perror("execlp");
        return 1;
    default:
        wait(NULL);
        printf("\n");
        printf("Some Text\n");
    }

    return 0;
}