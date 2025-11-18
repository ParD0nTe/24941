#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{

    pid_t pid = fork();

    if(pid == -1)
    {
        printf("Подпроцесс не создан");
    }
    else if(pid == 0)
    {
        printf("Подпроцесс %d\n",getpid());

        char **arg = malloc(argc*sizeof(char*));
        for(int i = 1; i < argc; i++)
        {   
            arg[i-1] = argv[i];
        }
        argv[argc-1] = NULL;
        execvp(argv[1],arg);
        perror("Ошибка выполнения cat");
        exit(1);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        printf("\nПодпроцесс завершён с кодом %d\n", WEXITSTATUS(status));
    }
}