#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <stdlib.h>

// ./server & sleep 0.1 && ./client

int main() {
    int server_sock;
    struct sockaddr_un addr;

    int clients[5];
    int client_id[5];

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

    printf("wait for client\n");
    fflush(stdout);

    fd_set read_fds;
    char buffer[257];
    int max_fd = server_sock;
    int client_count = 0;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);

        max_fd = server_sock;

        for (int i = 0; i < 5; i++) {
            if (clients[i] >= 0) {
                FD_SET(clients[i], &read_fds);
                if (clients[i] > max_fd)
                    max_fd = clients[i];
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(server_sock, &read_fds)) {
            int client_sock = accept(server_sock, NULL, NULL);
            if (client_sock < 0) {
                perror("accept");
                continue;
            }

            for (int i = 0; i < 5; i++) {
                if (clients[i] < 0) {
                    clients[i] = client_sock;

                    client_count++;
                    client_id[i] = client_count;

                    printf("new client added: client_%d\n",
                           client_count);
                    fflush(stdout);
                    break;
                }
            }
        }

        for (int i = 0; i < 5; i++) {
            int fd = clients[i];
            if (fd >= 0 && FD_ISSET(fd, &read_fds)) {

                ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
                if (bytes <= 0) {
                    printf("client_%d disconnected\n", client_id[i]);
                    close(fd);
                    clients[i] = -1;
                    client_id[i] = 0;
                    continue;
                }

                buffer[bytes] = '\0';

                for (int j = 0; j < bytes; j++)
                    buffer[j] = (char)toupper(buffer[j]);

                printf("client_%d: %s\n", client_id[i], buffer);
                fflush(stdout);
            }
        }
    }

    close(server_sock);
    unlink("/tmp/unix_socket");
    return 0;
}
