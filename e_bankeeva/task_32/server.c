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
int next_client_id = 1;

void timestamp(char *buf, size_t sz) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);

    strftime(buf, sz, "%Y-%m-%d %H:%M:%S", &tm_info);
    sprintf(buf + strlen(buf), ".%03ld", ts.tv_nsec / 1000000);
}

void handle_sigpoll(int sig)
{
    (void)sig;

    char ts[64], out[256];
    int fd;

    while ((fd = accept(server_fd, NULL, NULL)) >= 0) {

        fcntl(fd, F_SETFL, O_NONBLOCK);
        ioctl(fd, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

        for (int i = 0; i < 5; i++) {
            if (client_fd[i] == -1) {
                client_fd[i] = fd;
                client_num[i] = next_client_id++;

                timestamp(ts, sizeof(ts));
                printf("client_%d connected %s\n", client_num[i], ts);
                fflush(stdout);
                break;
            }
        }
    }

    for (int i = 0; i < 5; i++) {

        fd = client_fd[i];
        if (fd < 0) continue;

        char c;
        ssize_t r = read(fd, &c, 1);

        if (r > 0) {
            c = toupper((unsigned char)c);

            timestamp(ts, sizeof(ts));
            printf("client_%d: %s %c\n", client_num[i], ts, c);
            fflush(stdout);
        }
        else if (r == 0 || (r < 0 && errno != EAGAIN)) {

            timestamp(ts, sizeof(ts));
            printf("client_%d disconnected %s\n", client_num[i], ts);

            close(fd);
            client_fd[i] = -1;
            client_num[i] = 0;
        }
    }
}

int main()
{
    struct sockaddr_un addr;

    for (int i = 0; i < 5; i++)
        client_fd[i] = -1;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket"); return 1;
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen"); return 1;
    }

    printf("wait for client...");

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    ioctl(server_fd, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

    struct sigaction sa;
    sa.sa_handler = handle_sigpoll;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPOLL, &sa, NULL);

    printf("WAIT FOR CLIENTS...\n");

    while (1) pause();

    return 0;
}