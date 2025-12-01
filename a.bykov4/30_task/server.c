#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define SOCK_PATH "/tmp/upper.sock"

int main() {
    int srv = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);

    unlink(SOCK_PATH);
    bind(srv, (struct sockaddr*)&addr, sizeof(addr));
    listen(srv, 1);

    int cli = accept(srv, NULL, NULL);

    char buf[256];
    ssize_t n;

    while ((n = read(cli, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++)
            buf[i] = toupper((unsigned char)buf[i]);
        write(STDOUT_FILENO, buf, n);
    }

    close(cli);
    close(srv);
    return 0;
}
