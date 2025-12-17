#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define SOCKET_PATH "task32_socket"

int main() {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int iteration = 0;

    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;

        if (elapsed >= 5.0)
            break;

        int client_id = (iteration % 3) + 1;

        pid_t pid = fork();
        if (pid == 0) {
            // --- send_messages(client_id) начало ---
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock == -1)
                exit(1);

            struct sockaddr_un addr = {.sun_family = AF_UNIX};
            strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

            if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
                close(sock);
                exit(1);
            }

            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            double current_time = ts.tv_sec + ts.tv_nsec / 1e9;

            char msg[256];
            snprintf(msg, sizeof(msg), "Client%d message: at %.3f sec!", client_id, current_time);

            write(sock, msg, strlen(msg));
            close(sock);
            // --- send_messages(client_id) конец ---

            exit(0);
        }
        else if (pid > 0) {
            iteration++;
        }
        else {
            perror("fork");
            break;
        }
    }

    // Подождать завершения всех детей
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {}

    return 0;
}
