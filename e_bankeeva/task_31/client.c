#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "./socket"

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("usage: %s <1|2>\n", argv[0]);
        return 1;
    }

    int num = atoi(argv[1]);

    const char *msg;
    if (num == 1)
        msg = "hello";
    else
        msg = "goodbye";

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); return 1; }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("connect");
        close(fd);
        return 1;
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "client_%d: %s\n", num, msg);
    for (int i = 0; i < 30; i++) {
        snprintf(buf, sizeof(buf), "client_%d: %s\n", num, msg);
        write(fd, buf, strlen(buf));
        usleep(100000);
    }
    close(fd);

    close(fd);
    return 0;
}