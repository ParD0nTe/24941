#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <time.h>
#define SOCKET_PATH "./socket"

int main() {

    int client_fd[5];
    int client_active[5];
    struct timespec client_start[5];

    for (int i = 0; i < 5; i++) {
        client_fd[i] = -1;
        client_active[i] = 0;
    }

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    unlink(SOCKET_PATH);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        return 1;
    }
    printf("wait for client...\n");

    fd_set rfds;

    while (1) {

        FD_ZERO(&rfds);
        FD_SET(server_fd, &rfds);
        int max_fd = server_fd;

        for (int i = 0; i < 5; i++) {
            if (client_active[i]) {
                FD_SET(client_fd[i], &rfds);
                if (client_fd[i] > max_fd)
                    max_fd = client_fd[i];
            }
        }

        if (select(max_fd + 1, &rfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(server_fd, &rfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd < 0) continue;

            for (int i = 0; i < 5; i++) {
                if (!client_active[i]) {
                    client_fd[i] = fd;
                    client_active[i] = 1;
                    clock_gettime(CLOCK_MONOTONIC, &client_start[i]);

                    time_t t = time(NULL);
                    printf("client_%d: conected [%s]", i+1, ctime(&t));

                    break;
                }
            }
        }

        for (int i = 0; i < 5; i++) {

            if (client_active[i] && FD_ISSET(client_fd[i], &rfds)) {

                char buf[256];
                int r = read(client_fd[i], buf, sizeof(buf) - 1);

                if (r <= 0) {
                    struct timespec end;
                    clock_gettime(CLOCK_MONOTONIC, &end);

                    double diff =
                        (end.tv_sec - client_start[i].tv_sec) +
                        (end.tv_nsec - client_start[i].tv_nsec) / 1e9;

                    time_t t = time(NULL);
                    printf("client_%d disconected [%s(%.3f sec)]\n",
                           i+1, ctime(&t), diff);

                    close(client_fd[i]);
                    client_active[i] = 0;
                }
                else {
                    buf[r] = '\0';
                    buf[strcspn(buf, "\n")] = '\0';

                    for (int k = 0; k < r; k++)
                        buf[k] = toupper((unsigned char)buf[k]);

                    printf("%s\n", buf);
                    fflush(stdout);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}