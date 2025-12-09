#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stropts.h>
#include <errno.h>
#include <time.h>

#define SOCKET_PATH "./socket"

int server_fd;
int client_fd[5];
int client_num[5];
int next_id = 1;

struct timespec start_ts;
struct timespec end_ts;
int session_started = 0;

void print_runtime() {
    clock_gettime(CLOCK_MONOTONIC, &end_ts);

    double sec = (end_ts.tv_sec - start_ts.tv_sec)
               + (end_ts.tv_nsec - start_ts.tv_nsec) / 1e9;

    printf("\n(%.3f sec)\n", sec);
}

void handle_sigpoll(int sig)
{
    (void)sig;

    int fd;

    while ((fd = accept(server_fd, NULL, NULL)) >= 0) {

        fcntl(fd, F_SETFL, O_NONBLOCK);
        ioctl(fd, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

        for (int i = 0; i < 5; i++) {
            if (client_fd[i] == -1) {
                client_fd[i] = fd;
                client_num[i] = next_id++;

                if (!session_started) {
                    clock_gettime(CLOCK_MONOTONIC, &start_ts);
                    session_started = 1;
                }

                printf("client_%d connected\n", client_num[i]);
                fflush(stdout);
                break;
            }
        }
    }

    for (int i = 0; i < 5; i++)
    {
        int cfd = client_fd[i];
        if (cfd < 0) continue;

        char buf[256];
        ssize_t r = read(cfd, buf, sizeof(buf));

        if (r > 0) {
            buf[r] = '\0';

            for (int j = 0; j < r; j++)
                buf[j] = toupper((unsigned char)buf[j]);

            printf("client_%d %s\n", client_num[i], buf);
            fflush(stdout);
        }
        else if (r == 0 || (r < 0 && errno != EAGAIN)) {

            printf("client_%d disconnected\n", client_num[i]);

            close(cfd);
            client_fd[i] = -1;

            int active = 0;
            for (int k = 0; k < 5; k++)
                if (client_fd[k] != -1) active = 1;

            if (!active && session_started) {
                print_runtime();
                exit(0);
            }
        }
    }
}

int main()
{
    struct sockaddr_un addr;

    for (int i = 0; i < 5; i++)
        client_fd[i] = -1;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen"); return 1;
    }

    printf("wait for clients...\n");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    ioctl(server_fd, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

    struct sigaction sa = {0};
    sa.sa_handler = handle_sigpoll;
    sigaction(SIGPOLL, &sa, NULL);

    while (1) pause();
}