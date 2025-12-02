#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>       // toupper
#include <string.h>
#include <unistd.h>      // read, write, close, unlink
#include <sys/socket.h>  // socket, bind, listen, accept
#include <sys/un.h>      // sockaddr_un

#include <sys/epoll.h>
#include <fcntl.h>

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024
#define MAX_EVENTS 64

static int make_nonblock(int fd) {
    // запрос текущих флагов файла.
    // fcntl(..., F_GETFL) возвращает биты режима
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    // назначение новых флагов: те, что были + O_NONBLOCK
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void) {
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

    // epoll_create1 делает в ядре структуру-контейнер - место, куда ядро будет складывать события по сокетам
    int ep = epoll_create1(0);
    if (ep == -1) {
        perror("epoll_create1");
        exit(1);
    }

    // структура описыает инетерсующие события
    struct epoll_event ev;
    ev.events = EPOLLIN;      // мы хотим получать уведомления о возможности чтения
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

        // проходим по всем событиям, которые вернул epoll_wait
        for (int i = 0; i < nready; i++) {
            // получаем файловый дескриптор, на котором произошло событие
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // если событие произошло на основном серверном сокете (server_fd),
                // это означает, что кто-то хочет подключиться
                int cfd = accept(server_fd, NULL, NULL);
                if (cfd == -1) {
                    perror("accept");
                    continue;
                }

                // делаем клиентский сокет неблокирующим, чтобы он не тормозил программу
                make_nonblock(cfd);

                // создаём новое событие для epoll: будем следить за чтением  на этом сокете
                struct epoll_event cev;
                cev.events = EPOLLIN;
                cev.data.fd = cfd;

                // добавляем клиентский сокет в список отслеживаемых epoll.
                epoll_ctl(ep, EPOLL_CTL_ADD, cfd, &cev);
                continue; // Переходим к следующему событию.
            }

            // если событие произошло не на серверном сокете, значит это данные от клиента
            char buf[BUF_SIZE];
            ssize_t r = read(fd, buf, BUF_SIZE);

            if (r > 0) {
                for (ssize_t j = 0; j < r; j++) {
                    buf[j] = toupper((unsigned char)buf[j]);
                }
                write(STDOUT_FILENO, buf, r);
            } else if (r == 0) {
                // если клиент закрыл соединение
                // удаляем сокет из списка отслеживаемых epoll
                epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
            } else {
                // если произошла ошибка при чтении
                // тоже удаляем
                epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
            }
        }
    }

    // закрываем основной серверный сокет
    close(server_fd);
    // удаляем файл сокета
    unlink(SOCKET_PATH);

    return 0;
}
