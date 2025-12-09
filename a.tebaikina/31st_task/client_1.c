#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // read, write, close
#include <sys/socket.h>  // socket, connect
#include <sys/un.h>      // sockaddr_un
#include <time.h>        // rand

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024

int main(void) {
    int sock_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) ^ getpid());

    const char *msgs[] = {
            "AAA1 hello ",
            "AAA1 part ",
            "AAA1 segment "
    };
    int count_msgs = sizeof(msgs) / sizeof(msgs[0]);

    for (int i = 0; i < 5; i++) {
        const char *m = msgs[rand() % count_msgs];
        write(sock_fd, m, strlen(m));
        usleep((rand() % 300 + 100) * 1000);
    }

    while ((n = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sock_fd, buf, n) == -1) {
            perror("write");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (n == -1) perror("read");

    close(sock_fd);
    return 0;
}
