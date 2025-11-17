#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

void change_text(int fd_in) {
    char ch;
    while (read(fd_in, &ch, 1) > 0) {
        putchar(toupper(ch));
    }
    close(fd_in);
    printf("\n");
}

void give_text(int fd_out) {
    const char* text = "text to change";
    write(fd_out, text, strlen(text));
    close(fd_out);
}

int main() {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t cid = fork();

    if (cid == 0) {
        close(pipefd[1]);
        change_text(pipefd[0]);
        return 0;
    }
    else {
        close(pipefd[0]);
        give_text(pipefd[1]);
        wait(NULL);
    }

    return 0;
}