#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SOCKET_PATH "socket32"

void write_(int fd) {
    char buf[64];
    for (int i = 0; i < 10; i++) {
        const char* msg = "aaaaa";
        snprintf(buf, sizeof(buf), "%s\n", msg);
        write(fd, buf, strlen(buf));
    }
}

int main() {
    int fd;
    struct sockaddr_un address;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket"); 
        return 1; 
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("connect"); 
        close(fd); 
        return 1;
    }
        
    write_(fd);

    close(fd);
    return 0;
}