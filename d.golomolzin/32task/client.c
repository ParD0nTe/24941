#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SOCKET_PATH "./socket"

// низкая функция записи текста в сокет (без времени и задержки)
void write_(int fd, int num) {
    const char* texts[] = {
        "first", "second", "third", "fourth", "fifth",
        "sixth", "seventh", "eighth", "ninth", "tenth"
    };

    char buf[64];
    for (int i = 0; i < 10; i++) {
        const char* msg = texts[rand() % 10];
        snprintf(buf, sizeof(buf), "[%d] %s\n", num, msg);
        write(fd, buf, strlen(buf));
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int fd;
    struct sockaddr_un address;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) { perror("socket"); return 1; }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("connect"); close(fd); return 1;
    }

    int num = argc > 1 ? atoi(argv[1]) : 1;
    write_(fd, num);

    close(fd);
    return 0;
}
