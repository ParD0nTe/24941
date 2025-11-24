#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <poll.h>

#define SOCKET_PATH  "./socket"
#define BUF 1024
#define MAX 5

int main(){

    int server_fd, client_fd;
    struct sockaddr_un socket_addr;
    char buffer[BUF];
    ssize_t bytes;
    int count_ds = 1;

    struct pollfd fds[MAX + 1];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, SOCKET_PATH, sizeof(socket_addr.sun_path)-1);

    unlink(SOCKET_PATH);

    bind(server_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));

    listen(server_fd, 5);

    memset(fds, 0, sizeof(fds));

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        int ready = poll(fds, count_ds, -1);

        for (int i = 0; i < count_ds; i++) {
            if (fds[i].revents == 0) {
                continue;
            }

            if (fds[i].fd == server_fd) {
                if (fds[i].revents & POLLIN) {

                    client_fd = accept(server_fd, NULL, NULL);

                    if (count_ds < MAX + 1) {
                        fds[count_ds].fd = client_fd;
                        fds[count_ds].events = POLLIN;
                        count_ds++;
                        printf("Кто-то подключен\n");
                    } else {
                        printf("Достигнут лимит\n");
                        close(client_fd);
                    }
                }
            } 

            else if (fds[i].revents & POLLIN) {
                bytes = read(fds[i].fd, buffer, BUF - 1);
                
                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    
                    for (int j = 0; j < bytes; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    
                    printf("%s", buffer);
                    fflush(stdout);
                } 
                else if (bytes == 0) {
                    printf("Кто-то отключился\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                } 
            }
        }

        for (int i = 0; i < count_ds; i++) {
            if (fds[i].fd == -1) {
                for (int j = i; j < count_ds - 1; j++) {
                    fds[j] = fds[j + 1];
                }
                i--; 
                count_ds--;
            }
        }
    }

    for (int i = 0; i < count_ds; i++) {
        if (fds[i].fd != -1) {
            close(fds[i].fd);
        }
    }


    unlink(SOCKET_PATH);

    return 0;
}