#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>       // toupper
#include <string.h>
#include <unistd.h>      // read, write, close, unlink
#include <sys/socket.h>  // socket, bind, listen, accept
#include <sys/un.h>      // sockaddr_un

#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>         // timestamp
#include <errno.h>

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024
#define MAX_EVENTS 64

#define BLOCK_SIZE 4           // читаем порциями
#define MAX_CLIENTS 128

// структура клиента
typedef struct {
    int fd;
    int id; // для логов START/END
    char buf[BLOCK_SIZE]; // маленький рабочий буфер для набора BLOCK_SIZE байт
    int bufpos; // сколько байт уже накоплено в buf (0..BLOCK_SIZE)
    int active;// флаг: занятое место в пуле клиентов или свободное
} client_t;

static client_t clients[MAX_CLIENTS];

static int make_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// функция печати START/END
void print_time_event(int id, const char *event) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);  // запрашиваем текущее время

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info); // превращаем в норм вид

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info); // в строку
    printf("[%d] %s %s.%03ld\n", id, event, buf, ts.tv_nsec / 1000000);
    fflush(stdout);
}

// выделение нового ID
int alloc_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            clients[i].active = 1;
            clients[i].bufpos = 0;
            clients[i].id = i + 1;
            return i;
        }
    }
    return -1;
}

int main(void) {
    memset(clients, 0, sizeof(clients));

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        exit(1);
    }

    make_nonblock(server_fd);

    int ep = epoll_create1(0);
    if (ep == -1) {
        perror("epoll_create1");
        exit(1);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(ep, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl server_fd");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];

    printf("сервер запущен (epoll)\n");

    for (;;) {
        int nready = epoll_wait(ep, events, MAX_EVENTS, -1);
        if (nready == -1) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {

                int cfd = accept(server_fd, NULL, NULL);
                if (cfd == -1) {
                    perror("accept");
                    continue;
                }

                make_nonblock(cfd);

                int slot = alloc_client_slot();
                if (slot == -1) {
                    close(cfd);
                    continue;
                }

                clients[slot].fd = cfd;

                print_time_event(clients[slot].id, "START");

                struct epoll_event cev;
                cev.events = EPOLLIN;
                cev.data.fd = cfd;

                epoll_ctl(ep, EPOLL_CTL_ADD, cfd, &cev);
                continue;
            }

            // чтение данных от клиентов

            char rbuf[BLOCK_SIZE];
            ssize_t r = read(fd, rbuf, BLOCK_SIZE);   // читаем ровно BLOCK_SIZE

            int slot = -1;
            for (int k = 0; k < MAX_CLIENTS; k++) {
                if (clients[k].active && clients[k].fd == fd) {
                    slot = k;
                    break;
                }
            }

            if (slot < 0) continue;

            client_t *cl = &clients[slot];

            if (r > 0) {
                for (ssize_t j = 0; j < r; j++) {
                    char c = toupper((unsigned char)rbuf[j]);
                    cl->buf[cl->bufpos++] = c;

                    if (cl->bufpos == BLOCK_SIZE) {           // изменено
                        printf("[%d] %.*s\n", cl->id, BLOCK_SIZE, cl->buf);  // изменено: печать с ID
                        fflush(stdout);                        // изменено: чтобы вывод был сразу
                        cl->bufpos = 0;                        // оставляем как есть
                    }

                }

            } else if (r == 0) {

                if (cl->bufpos > 0) {
                    write(STDOUT_FILENO, cl->buf, cl->bufpos);
                }

                print_time_event(cl->id, "END");

                epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                cl->active = 0;

            } else {
                epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                cl->active = 0;
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
