#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#define SOCKET_PATH "./socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    unlink(SOCKET_PATH);
    
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    listen(server_fd, 5);
    
    printf("Сервер запущен\n");
    
    client_fd = accept(server_fd, NULL, NULL);
    
    printf("Кто-то подключен\n");
    
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';

        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        
        printf("%s", buffer);
        fflush(stdout);
    }
    
    if (bytes_read == -1) {
        perror("read");
    }
    
    printf("\nКто-то отключился\n");
    
    close(client_fd);
    close(server_fd);
    
    unlink(SOCKET_PATH);
    
    return 0;
}