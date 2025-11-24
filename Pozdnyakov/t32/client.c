#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket"
#define BUF 1024

int main(){

    int client_fd;
    struct sockaddr_un socket_addr;
    char buffer[BUF];
    ssize_t bytes;


    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, SOCKET_PATH, sizeof(socket_addr.sun_path)-1);

    connect(client_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));

    while((bytes = read(STDIN_FILENO, buffer, BUF)) > 0){
        write(client_fd, buffer, bytes);
    }

    close(client_fd);

    return 0;
}