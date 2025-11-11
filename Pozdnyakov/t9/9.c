#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid = fork();

    if(pid == -1){
        printf("Подпроцесс не создан");
    }
    else if(pid == 0)
    {
        printf("Подпроцесс %d\n", getpid());
        execl("/bin/cat", "cat", "file.txt", NULL);
        perror("Ошибка выполнения");
        return 1;
    }
    else
    {   
        wait(NULL);
        printf("\nПодпроцесс завершён\n");
    }



    return 0;
}