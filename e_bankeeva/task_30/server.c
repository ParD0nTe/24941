#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>


int main()
{
    struct sockaddr_un server_addr, client_addr;

    const int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
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

    if (listen(server_sock, 1) < 0) {
        perror("listen");
        close(server_sock);
        unlink("/tmp/unix_socket");
        return 1;
    }

    socklen_t client_len = sizeof(client_addr);
    const int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
        perror("accept");
        close(server_sock);
        unlink("/tmp/unix_socket");
        return 1;
    }

    printf("connected\n");
    char buffer[257];

    while (1) {
        const ssize_t bytes_received = recv(client_sock, buffer, 256, 0);

        if (bytes_received <= 0)
            break;

        buffer[bytes_received] = '\0';

        for (int i = 0; i < bytes_received; i++)
            buffer[i] = toupper((unsigned char)buffer[i]);

        printf("received: %s\n", buffer);
    }

    close(client_sock);
    close(server_sock);
    unlink("/tmp/unix_socket");

    return 0;
}