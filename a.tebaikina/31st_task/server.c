#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>       // toupper
#include <string.h>
#include <unistd.h>      // read, write, close, unlink
#include <sys/socket.h>  // socket, bind, listen, accept
#include <sys/un.h>      // sockaddr_un
#include <time.h>

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024


typedef struct {
    int fd;                // файловый дескриптор клиента
    int active;            // признак, что клиент существует
    int id;                // номер клиента (1..n)
    struct timespec start; // время начала работы клиента
} client_t;

client_t clients[FD_SETSIZE]; // массив клиентов как у тебя, но расширенный

int main(void) {
    int server_fd;

    // ! много клиентов - все нули !
    int client_fds[FD_SETSIZE]; // оставляем, хотя теперь не основной массив
    int client_count = 0;
    for (int i = 0; i < FD_SETSIZE; i++) {
        client_fds[i] = -1;
        clients[i].active = 0;
        clients[i].id = i + 1;
    }

    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n; // сколько байт прочитали

    // 1. создаем сокет
    // AF_UNIX  → сокет для локального общения через файл (не через интернет)
    // SOCK_STREAM → потоковое соединение, как TCP (надёжная передача байтов)
    // 0 → использовать протокол по умолчанию для такого типа сокета
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. заполяем структуру addr( там структура, тип сокета+путь к файлу сокета)
    memset(&addr, 0, sizeof(addr)); // обнуляем весь addr
    addr.sun_family = AF_UNIX;
    // Копируем путь к сокету
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    // 3. привязываем сокет к пути в файловой системе
    // bind - прикрепить сокет к адресу
    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. начинаем слушать
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("сервер запущен, ждет подключения\n");

    while (1) {
        // Для управления fd_set используются макросы:
        // FD_ZERO() для инициализации,
        // FD_SET() для добавления дескриптора,
        fd_set readfds; // набор дескрипторов, за которыми будет наблюдать select()
        FD_ZERO(&readfds);

        // добавляем серверный сокет в список наблюдаемых
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        // следим за клиентскими сокетами
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (clients[i].active) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > max_fd)
                    max_fd = clients[i].fd;
            }
        }

        // ------- 5. ждём активности на любом сокете -------
        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            break;
        }

        // ------- 6. подключение клиента ------
        if (FD_ISSET(server_fd, &readfds)) {
            int new_fd = accept(server_fd, NULL, NULL);
            if (new_fd == -1) {
                perror("accept");
            } else {
                int placed = 0;
                for (int i = 0; i < FD_SETSIZE; i++) {
                    if (!clients[i].active) {
                        clients[i].active = 1;
                        clients[i].fd = new_fd;
                        clock_gettime(CLOCK_MONOTONIC, &clients[i].start); // время старта
                        printf("[SERVER] START client %d (fd=%d)\n", clients[i].id, new_fd);
                        placed = 1;
                        break;
                    }
                }
                if (!placed) {
                    printf("Слишком много клиентов!\n");
                    close(new_fd);
                }
            }
        }

        // ------- 7. тексты от клиентов ---------
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (clients[i].active && FD_ISSET(clients[i].fd, &readfds)) {

                char c;
                ssize_t n = read(clients[i].fd, &c, 1);

                if (n > 0) {
                    c = toupper((unsigned char)c);
                    write(STDOUT_FILENO, &c, 1);
                }
                else {
                    // клиент отключился
                    struct timespec end;
                    clock_gettime(CLOCK_MONOTONIC, &end);

                    double diff =
                            (end.tv_sec - clients[i].start.tv_sec) +
                            (end.tv_nsec - clients[i].start.tv_nsec) / 1e9;

                    printf("\n[SERVER] END client %d (fd=%d), alive %.6f sec\n",
                           clients[i].id, clients[i].fd, diff);

                    close(clients[i].fd);
                    clients[i].active = 0;
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
