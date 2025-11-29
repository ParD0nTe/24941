#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


int main() {
    const int client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "/tmp/unix_socket", sizeof(server_addr.sun_path) - 1);

    printf("connecting\n");
    fflush(stdout);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_sock);
        return 1;
    }

    printf("connected\n");
    fflush(stdout);
    usleep(200000);

    char buffer[256];

    while (1) {
        usleep(200000);
        printf("enter text: ");
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin))
            break;

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "q") == 0)
            break;

        if (send(client_sock, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
    }

    close(client_sock);

    return 0;
}
