#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

volatile sig_atomic_t timeout = 0;

void timeout_handler(int sig) { (void)sig; timeout = 1; }

int main(void) {
    int fd = open("input.txt", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    char buffer[4096];
    ssize_t nread = read(fd, buffer, sizeof buffer);
    if (nread < 0) { perror("read"); return 1; }

    int lines = 1;
    for (ssize_t i = 0; i < nread; ++i) if (buffer[i] == '\n') ++lines;

    int pos_sz = lines + 1;
    int pos[pos_sz];
    pos[0] = 0;
    int idx = 1;
    for (ssize_t i = 0; i < nread; ++i)
        if (buffer[i] == '\n') pos[idx++] = (int)i + 1;
    pos[lines] = (int)nread;

    struct sigaction sa = {0};
    sa.sa_handler = timeout_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) { perror("sigaction"); return 1; }

    for (;;) {
        timeout = 0;
        alarm(5);

        printf("Введите номер строки (1-%d): ", lines);
        fflush(stdout);

        char inbuf[32];
        ssize_t r = read(STDIN_FILENO, inbuf, sizeof inbuf - 1);
        if (r < 0) {
            if (errno == EINTR && timeout) {
                printf("\n");
                printf("----------------------------------------------------------------\n");
                write(STDOUT_FILENO, buffer, nread);
                write(STDOUT_FILENO, "\n", 1);
                break;
            } else {
                perror("read(stdin)");
                break;
            }
        }

        alarm(0);
        inbuf[r] = '\0';
        int num = atoi(inbuf);

        if (num == 0) break;
        if (num < 1 || num > lines) {
            puts("Нет такой строки.");
            continue;
        }

        int start = pos[num - 1];
        int len   = pos[num] - pos[num - 1];

        if (lseek(fd, start, SEEK_SET) == -1) { perror("lseek"); break; }

        char out[len];
        if (read(fd, out, len) != len) { perror("read(line)"); break; }
        write(STDOUT_FILENO, out, len);
        if (len == 0 || out[len-1] != '\n') write(STDOUT_FILENO, "\n", 1);
    }
}
