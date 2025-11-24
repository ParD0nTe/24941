#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

#define SOCKET_PATH  "./socket"
#define BUF 1024
#define MAX 5

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un socket_addr;
    char buffer[BUF];
    ssize_t bytes;
    int count_ds = 1;

    struct pollfd fds[MAX + 1];

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    set_nonblocking(server_fd);

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, SOCKET_PATH, sizeof(socket_addr.sun_path)-1);

    unlink(SOCKET_PATH);

    bind(server_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr));
    listen(server_fd, 5);

    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    printf("Сервер запущен...\n");

    while (1) {
        int ready = poll(fds, count_ds, -1);

        for (int i = 0; i < count_ds; i++) {
            if (fds[i].revents == 0) continue;

            if (fds[i].fd == server_fd) {
                if (fds[i].revents & POLLIN) {
                    while (1) {
                        client_fd = accept(server_fd, NULL, NULL);
                        if (client_fd == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break;
                            }
                        }
                        
                        if (count_ds < MAX + 1) {
                            set_nonblocking(client_fd);
                            fds[count_ds].fd = client_fd;
                            fds[count_ds].events = POLLIN;
                            count_ds++;
                            printf("Новое подключение\n");
                        } else {
                            printf("Лимит отказываем\n");
                            close(client_fd);
                        }
                    }
                }
            } 
            else if (fds[i].revents & POLLIN) {
                while (1) {
                    bytes = read(fds[i].fd, buffer, BUF - 1);
                    
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        
                        for (int j = 0; j < bytes; j++) {
                            buffer[j] = toupper(buffer[j]);
                        }
                        
                        printf("%s", buffer);
                        fflush(stdout);
                        continue;
                    } 
                    else if (bytes == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                    }
                    else if (bytes == 0) {
                        printf("Кто-то отключился\n");
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        break;
                    }
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