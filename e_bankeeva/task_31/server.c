#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>

// запускать через ./server && ./client но чтоб красиво было задержку добавить
int main()
{
    struct sockaddr_un server_addr, client_addr;

    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    unlink("/tmp/unix_socket");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "/tmp/unix_socket", sizeof(server_addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_sock);
        return 1;
    }

    printf("wait for client\n");
    fflush(stdout);
    usleep(300000);

    if (listen(server_sock, 10) < 0) {
        perror("listen");
        close(server_sock);
        exit(1);
    }

    int client_sock;
    int client_socks[10] = {0};
    time_t client_time[10] = {0};
    fd_set read_fds;
    int max_sd, activity;
    socklen_t client_len;
    char buffer[257];

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);
        max_sd = server_sock;

        for (int i = 0; i < 10; i++) {
            if (client_socks[i] > 0) {
                FD_SET(client_socks[i], &read_fds);
                if (client_socks[i] > max_sd)
                    max_sd = client_socks[i];
            }
        }

        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        if (FD_ISSET(server_sock, &read_fds)) {
            client_len = sizeof(client_addr);
            if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                perror("accept");
                continue;
            }

            int placed = 0;
            for (int j = 0; j < 10; j++) {
                if (client_socks[j] == 0) {
                    client_socks[j] = client_sock;
                    client_time[j] = time(NULL);
                    char *time_str = ctime(&client_time[j]);
                    usleep(300000);
                    printf("new client connected\n");
                    fflush(stdout);
                    placed = 1;
                    break;
                }
            }

            if (!placed) {
                printf("too many clients\n");
                fflush(stdout);
                close(client_sock);
            }
        }

        for (int i = 0; i < 10; i++) {
            const int sd = client_socks[i];
            if (sd > 0 && FD_ISSET(sd, &read_fds)) {
                const ssize_t bytes_received = recv(sd, buffer, 257, 0);
                if (bytes_received <= 0) {
                    char *time_str = ctime(&client_time[i]);
                    time_str[strcspn(time_str, "\n")] = '\0';
                    printf("client_%d disconnected, connected at %s\n", sd - 3, time_str);
                    fflush(stdout);
                    usleep(200000);
                    close(sd);
                    client_socks[i] = 0;
                    client_time[i] = 0;
                } else {
                    buffer[bytes_received] = '\0';
                    for (int j = 0; j < bytes_received; j++)
                        buffer[j] = toupper((unsigned char)buffer[j]);
                    char *time_str = ctime(&client_time[i]);
                    time_str[strcspn(time_str, "\n")] = '\0';
                    printf("[%s] client_%d: %s\n", time_str, sd - 3, buffer);
                    fflush(stdout);
                }
            }
        }
    }


    for (int i = 0; i < 10; i++)
        if (client_socks[i] > 0)
            close(client_socks[i]);
    close(server_sock);
    unlink("/tmp/unix_socket");

    return 0;
}