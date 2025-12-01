#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

int server_sock;
int clients[5];
int client_id[5];
int client_count = 0;

// ./server & sleep 0.1 && ./client

void handle_sigio(int sig)
{
    int client_sock = accept(server_sock, NULL, NULL);
    if (client_sock >= 0) {
        fcntl(client_sock, F_SETFL, O_NONBLOCK | O_ASYNC);
        fcntl(client_sock, F_SETOWN, getpid());

        for (int i = 0; i < 5; i++) {
            if (clients[i] < 0) {
                clients[i] = client_sock;
                client_id[i] = ++client_count;

                printf("new client added: client_%d (fd=%d)\n",
                       client_id[i], client_sock);
                fflush(stdout);
                break;
            }
        }
    }

    char buffer[256];
    for (int i = 0; i < 5; i++) {
        int fd = clients[i];
        if (fd < 0) continue;

        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {

            buffer[bytes] = '\0';
            for (int j = 0; j < bytes; j++)
                buffer[j] = (char)toupper(buffer[j]);

            printf("client_%d: %s\n", client_id[i], buffer);
            fflush(stdout);

        } else if (bytes == 0) {
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

    fcntl(server_sock, F_SETFL, O_NONBLOCK | O_ASYNC);
    fcntl(server_sock, F_SETOWN, getpid());

    struct sigaction sa;
    sa.sa_handler = handle_sigio;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGIO, &sa, NULL);

    printf("wait for client\n");
    fflush(stdout);

    while (1) pause();

    close(server_sock);
    unlink("/tmp/unix_socket");

    return 0;
}