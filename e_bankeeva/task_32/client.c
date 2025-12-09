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
    const char *msg = (num == 1 ? "hello" : "goodbye");

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path)-1);

    connect(fd, (struct sockaddr*)&address, sizeof(address));

    int len = strlen(msg);

    for (int repeat = 0; repeat < 5; repeat++) {
        write(fd, msg, len);
        usleep(100000);
    }

    close(fd);
    return 0;
}