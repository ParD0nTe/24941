#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>

int main()
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    { // Ребёнок
        close(pipefd[1]); // Закрываем запись

        char c;
        while (read(pipefd[0], &c, 1) > 0)
        {
            putchar(toupper(c));
        }
        close(pipefd[0]);
        exit(0);

    } else
    { // Родитель
        close(pipefd[0]); // Закрываем чтение

        const char *text = "hello world from parent!\n";
        write(pipefd[1], text, strlen(text));
        close(pipefd[1]); // Закрываем запись — сигнал конца для ребёнка

        wait(NULL); // Ждём завершения ребёнка
        exit(0);
    }
}