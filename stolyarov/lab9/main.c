#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    // В дочернем процессе
    if (pid == 0) 
    {
        // Исполняем команду cat для файла text.txt
        execlp("cat", "cat", "text.txt", (char *)NULL);
        perror("execlp");
        return 1;
    }
    else
    {
        wait(NULL);
        printf("\nКонец\n");
    }
}
