#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#define SOCKET_PATH "task32_socket"
#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048
#define MAX_EVENTS 100

// состояние клиента (не менять имя структуры и массива)
typedef struct {
    int fd;
    int messages_received;
    char buf[BUFFER_SIZE];
    size_t buf_len;
} client_state_t;

client_state_t clients[MAX_CLIENTS];


// ===== вспомогательные функции (имена заменены) =====

// очистка таблицы клиентов
void reset_client_table() {
    for (int k = 0; k < MAX_CLIENTS; ++k) {
        clients[k].fd = -1;
        clients[k].messages_received = 0;
        clients[k].buf_len = 0;
    }
}

// поиск свободного места в массиве
int acquire_slot() {
    for (int p = 0; p < MAX_CLIENTS; ++p) {
        if (clients[p].fd == -1)
            return p;
    }
    return -1;
}

// поиск по fd
int match_fd(int handle) {
    for (int z = 0; z < MAX_CLIENTS; ++z) {
        if (clients[z].fd == handle)
            return z;
    }
    return -1;
}



// ===============  MAIN  ===============
int main() {
    unlink(SOCKET_PATH);

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Порядок bind+listen слегка переставлен местами
    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s\n", SOCKET_PATH);

    reset_client_table();

    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    struct epoll_event base_ev = {0};
    base_ev.events = EPOLLIN;
    base_ev.data.fd = server_socket;

    // основной epoll-контрол
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &base_ev) == -1) {
        perror("epoll_ctl: server_socket");
        close(server_socket);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    // ===== таймер работы =====
    struct timespec t0;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    const double ACTIVE_TIME = 5.0;
    int total_messages = 0;


    // ========== главный цикл событий ==========
    while (1) {
        struct timespec tnow;
        clock_gettime(CLOCK_MONOTONIC, &tnow);

        double dt = (tnow.tv_sec - t0.tv_sec) +
                    (tnow.tv_nsec - t0.tv_nsec) / 1e9;

        if (dt >= ACTIVE_TIME)
            break;

        int time_left = (int)((ACTIVE_TIME - dt) * 1000);
        if (time_left <= 0)
            break;

        struct epoll_event ready[MAX_EVENTS];
        int hits = epoll_wait(epoll_fd, ready, MAX_EVENTS, time_left);

        if (hits == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < hits; ++i) {
            int current_fd = ready[i].data.fd;

            // ======= новое подключение =======
            if (current_fd == server_socket) {
                int newcomer = accept4(server_socket, NULL, NULL, SOCK_NONBLOCK);
                if (newcomer == -1) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        perror("accept4");
                    continue;
                }

                int slot = acquire_slot();
                if (slot < 0) {
                    close(newcomer);
                    continue;
                }

                clients[slot].fd = newcomer;
                clients[slot].buf_len = 0;
                clients[slot].messages_received = 0;

                struct epoll_event cev = {0};
                cev.events = EPOLLIN | EPOLLRDHUP;
                cev.data.fd = newcomer;

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newcomer, &cev) == -1) {
                    perror("epoll_ctl: add client");
                    close(newcomer);
                    clients[slot].fd = -1;
                }
            }

            // ======= данные от существующего клиента =======
            else {
                int idx = match_fd(current_fd);
                if (idx < 0) {
                    close(current_fd);
                    continue;
                }

                char incoming[BUFFER_SIZE];
                ssize_t r = read(current_fd, incoming, sizeof(incoming));

                if (r <= 0) {
                    // если были данные в буфере — вывести их
                    if (clients[idx].buf_len > 0) {
                        for (size_t a = 0; a < clients[idx].buf_len; ++a)
                            clients[idx].buf[a] =
                                toupper((unsigned char)clients[idx].buf[a]);

                        write(STDOUT_FILENO,
                              clients[idx].buf,
                              clients[idx].buf_len);

                        clients[idx].messages_received++;
                    }

                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    close(current_fd);
                    clients[idx].fd = -1;
                    continue;
                }

                // нормальные данные
                if (clients[idx].buf_len + r < BUFFER_SIZE) {
                    memcpy(clients[idx].buf + clients[idx].buf_len, incoming, r);
                    clients[idx].buf_len += r;
                } else {
                    clients[idx].buf_len = 0; // сбрасываем переполнение
                }

                // обработка до '!'
                while (1) {
                    char *mark = memchr(clients[idx].buf, '!', clients[idx].buf_len);
                    if (!mark) break;

                    size_t segment_len = mark - clients[idx].buf + 1;

                    for (size_t c = 0; c < segment_len; ++c)
                        clients[idx].buf[c] =
                            toupper((unsigned char)clients[idx].buf[c]);

                    write(STDOUT_FILENO, clients[idx].buf, segment_len);
                    total_messages++;

                    memmove(clients[idx].buf,
                            clients[idx].buf + segment_len,
                            clients[idx].buf_len - segment_len);

                    clients[idx].buf_len -= segment_len;
                }
            }
        }
    }

    // ===== завершение =====
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double total_time = (t1.tv_sec - t0.tv_sec) +
                        (t1.tv_nsec - t0.tv_nsec) / 1e9;

    for (int m = 0; m < MAX_CLIENTS; ++m) {
        if (clients[m].fd != -1)
            close(clients[m].fd);
    }

    close(server_socket);
    close(epoll_fd);
    unlink(SOCKET_PATH);

    printf("\n--- Server finished after %.3f seconds ---\n", total_time);
    printf("Total messages processed: %d\n", total_messages);

    return 0;
}
