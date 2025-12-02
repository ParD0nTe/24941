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


int server_sock;
int clients[5];
int client_id[5];
int client_count = 0;

// ./server & sleep 0.1 && ./client

void timestamp(char *buf, size_t sz) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(buf, sz, "[%Y-%m-%d %H:%M:%S]", &tm);
}

void start_client(const char *prog) {
    pid_t pid = fork();
    if (pid == 0) {
        execl(prog, prog, NULL);
        perror("execl");
        exit(1);
    }
}

void handle_sigpoll(int sig)
{
    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock >= 0) {
        fcntl(client_sock, F_SETFL, O_NONBLOCK);
        ioctl(client_sock, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

        for (int i = 0; i < 5; i++) {
            if (clients[i] < 0) {
                clients[i] = client_sock;
                client_id[i] = ++client_count;

                char ts[64];
                timestamp(ts, sizeof(ts));

                char msg[128];
                int len = snprintf(msg, sizeof(msg),
                                   "%s new client added: client_%d (fd=%d)\n",
                                   ts, client_id[i], client_sock);
                write(1, msg, len);
                break;
            }
        }
    }

    char buffer[256];
    for (int i = 0; i < 5; i++) {
        int fd = clients[i];
        if (fd < 0) continue;

        ssize_t bytes = read(fd, buffer, 1);
        if (bytes > 0) {

            buffer[bytes] = '\0';
            for (int j = 0; j < bytes; j++)
                buffer[j] = (char)toupper(buffer[j]);

            char ts[64];
            timestamp(ts, sizeof(ts));

            char msg[128];
            int len = snprintf(msg, sizeof(msg),
                                   "%s new client added: client_%d (fd=%d)\n", // added timestamp
                                   ts, client_id[i], client_sock);
            write(1, msg, len);

        } else if (bytes == 0 || (bytes < 0 && errno != EAGAIN)) {
            printf("client_%d disconnected\n", client_id[i]);
            close(fd);
            clients[i] = -1;
            client_id[i] = 0;
        }
    }
}

int main() {

    struct sockaddr_un addr;

    for (int i = 0; i < 5; i++) {
        clients[i] = -1;
        client_id[i] = 0;
    }

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    unlink("/tmp/unix_socket");

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/unix_socket", sizeof(addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, 10) < 0) {
        perror("listen");
        close(server_sock);
        unlink("/tmp/unix_socket");
        return 1;
    }

    fcntl(server_sock, F_SETFL, O_NONBLOCK);
    ioctl(server_sock, I_SETSIG, S_INPUT | S_HANGUP | S_ERROR);

    struct sigaction sa;
    sa.sa_handler = handle_sigpoll;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPOLL, &sa, NULL);

    printf("wait for client\n");
    fflush(stdout);

    start_client("./client1");
    start_client("./client2");

    while (1) pause();

    close(server_sock);
    unlink("/tmp/unix_socket");

    return 0;
}
