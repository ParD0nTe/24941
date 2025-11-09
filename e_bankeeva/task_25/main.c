#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>


int main(const int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("\n");
        return 1;
    }

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        printf("Error pipe()\n");
        return -1;
    }

    const pid_t child = fork();

    if (child == -1)
    {
        printf("Error fork()\n");
        return 1;
    }

    if (child == 0)
    {
        close(pipe_fd[1]);
        char buf[256];
        const ssize_t bytes_read = read(pipe_fd[0], buf, 256);

        for (int i = 0; i < bytes_read; i++)
        {
            buf[i] = (char)toupper(buf[i]);
        }
        buf[bytes_read] = '\0';

        printf("переделано: %s\n", buf);

        close(pipe_fd[0]);
    }
    else
    {
        close(pipe_fd[0]);

        const char *text = argv[1];
        write(pipe_fd[1], text, strlen(text));
        close(pipe_fd[1]);

        wait(NULL);
    }

    return 0;
}
