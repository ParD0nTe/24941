#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "ctype.h"
int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[256];
    pipe(pipefd);
    pid = fork();
    if (pid == 0) {
    
        close(pipefd[1]);
        int nbytes = read(pipefd[0], buffer, sizeof(buffer));
        for(int i=0;i<nbytes;i++){
            buffer[i]=toupper(buffer[i]);
        }
        if (nbytes > 0) {
            printf("%s\n", buffer);
        }

        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[0]);
        
        char *message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do ";
        write(pipefd[1], message, strlen(message) + 1);
        
        close(pipefd[1]);
        wait(NULL);
    }
    
    return 0;
}