#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("\n");
        return 1;
    }

    const pid_t child = fork();

    if (child == -1)
    {
        printf("failed to fork");
        return 1;
    }

    if (child == 0)
    {
        execlp("cat", "cat", argv[1], NULL);
        perror("execlp");
        return 1;
    }
    else
    {
        int status;
        const pid_t wp = waitpid(child, &status, 0);
        printf("\n");

        if (wp == -1) {
            perror("failed to waited");
            return 1;
        }
        printf("child process terminated\n");
    }

    return 0;
}
