#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

int main()
{
    int pipefd[2];
    int nbytes;

    pipe(pipefd);

    pid_t pid = fork();

    char buf[100];

    if(pid == 0)
    {
        close(pipefd[1]);
        while((nbytes = read(pipefd[0],buf,sizeof(buf)) )> 0){

            for(int i = 0; i < nbytes; i++)
            {
                buf[i] = toupper(buf[i]);
            }

            for(int i = 0; i < nbytes; i++)
            {
                printf("%c", buf[i]);
            }

        }
        close(pipefd[0]);
        return 0;
    }
    else
    {
        close(pipefd[0]);
        char *arr[] = {"kjfdnbksjnfbnkfbnagjargajgirjmger",NULL};
        for(int i = 0; arr[i]!=NULL; i++){
            write(pipefd[1],arr[i],strlen(arr[i]));
        }   
        close(pipefd[1]);
        wait(NULL);
    }
}