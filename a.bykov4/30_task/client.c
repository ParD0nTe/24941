#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define S_PATH "./socket"

int main(int argc, char* argv[]) {
    srand(time(NULL));

    const char* months[] = {
        "January", "February", "March", "April", "May",
        "June", "July", "August", "September", "October"
    };

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, S_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    int tag = (argc > 1) ? atoi(argv[1]) : 1;

    char out[64];
    for (int i = 0; i < 10; i++) {
        const char* m = months[rand() % 10];
        snprintf(out, sizeof(out), "[%d] %s\n", tag, m);
        write(sock, out, strlen(out));
    }

    close(sock);
    return 0;
}

