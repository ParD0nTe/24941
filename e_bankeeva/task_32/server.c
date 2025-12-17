#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <aio.h>
#include <signal.h>
#include <time.h>

#define SOCKET_PATH "./socket"

struct client {
    int fd;
    struct aiocb aio;
    char buf[256];
    int active;
    int id;
};

struct client clients[5];
int active_clients = 0;

struct timespec session_start, session_end;
int session_started = 0;


void aio_callback(union sigval sigval) {
    struct client *c = sigval.sival_ptr;
    int r = aio_return(&c->aio);

    if (r <= 0) {
        close(c->fd);
        c->active = 0;
        active_clients--;

        time_t t = time(NULL);
        printf("client disconnected [%s]\n", ctime(&t));

        if (active_clients == 0 && session_started) {
            clock_gettime(CLOCK_MONOTONIC, &session_end);
            double diff = (session_end.tv_sec - session_start.tv_sec) +
                          (session_end.tv_nsec - session_start.tv_nsec) / 1e9;
            printf("(%.3f sec)\n", diff);
            session_started = 0;
        }
        fflush(stdout);
        return;
    }

    c->buf[r] = 0;
    c->buf[strcspn(c->buf, "\n")] = 0;

    for (int i = 0; i < r; i++)
        c->buf[i] = toupper((unsigned char)c->buf[i]);

    printf("client_%d: %s\n", c->id, c->buf);
    fflush(stdout);

    memset(&c->aio, 0, sizeof(struct aiocb));
    c->aio.aio_fildes = c->fd;
    c->aio.aio_buf = c->buf;
    c->aio.aio_nbytes = 256;
    c->aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
    c->aio.aio_sigevent.sigev_notify_function = aio_callback;
    c->aio.aio_sigevent.sigev_value.sival_ptr = c;

    if (aio_read(&c->aio) < 0)
        perror("aio_read");
}


int main() {

    for (int i = 0; i < 5; i++)
        clients[i].active = 0;

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

    while (1) {
        int fd = accept(server_fd, NULL, NULL);
        if (fd < 0) continue;

        int idx = -1;
        for (int i = 0; i < 5; i++)
            if (!clients[i].active) { idx = i; break; }

        if (idx < 0) {
            close(fd);
            continue;
        }

        struct client *c = &clients[idx];
        c->fd = fd;
        c->active = 1;
        c->id = idx + 1;

        if (active_clients == 0) {
            clock_gettime(CLOCK_MONOTONIC, &session_start);
            session_started = 1;
        }
        active_clients++;

        time_t t = time(NULL);
        printf("client_%d connected [%s]\n", idx+1, ctime(&t));

        memset(&c->aio, 0, sizeof(struct aiocb));
        c->aio.aio_fildes = fd;
        c->aio.aio_buf = c->buf;
        c->aio.aio_nbytes = 256;
        c->aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
        c->aio.aio_sigevent.sigev_notify_function = aio_callback;
        c->aio.aio_sigevent.sigev_value.sival_ptr = c;

        if (aio_read(&c->aio) < 0)
            perror("aio_read");
    }

    close(server_fd);
    return 0;
}