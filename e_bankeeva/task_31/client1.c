#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUF 1024

int main() {
    int client_fd;
    struct sockaddr_un socket_addr;

    char buffer[BUF] = "hello";
    ssize_t bytes = strlen(buffer);

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, SOCKET_PATH, sizeof(socket_addr.sun_path)-1);

    connect(client_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));

    usleep(300000);

    for (int i = 0; i < 60; i++) {
        write(client_fd, buffer, bytes);
        usleep(100000);
    }

    close(client_fd);
    return 0;
}