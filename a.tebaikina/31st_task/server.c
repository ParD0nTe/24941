#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>       // toupper
#include <string.h>
#include <unistd.h>      // read, write, close, unlink
#include <sys/socket.h>  // socket, bind, listen, accept
#include <sys/un.h>      // sockaddr_un

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024

int main(void) {
    int server_fd;
    // ! много клиентов - все нули !
    int client_fds[FD_SETSIZE];
    int client_count = 0;
    for (int i = 0; i < FD_SETSIZE; i++)
        client_fds[i] = -1;
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

    // 3. привязываем сокет к пути в файловой системе
    // bind - прикрепить сокет к адресу
    if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. начинаем слушать ! увеличили очередь для 31го !
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("сервер запущен, ждет подключения");

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
            if (client_fds[i] != -1) {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd)
                    max_fd = client_fds[i];
            }
        }

        // ------- 5. ждём активности на любом сокете -------
        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            break;
        }

        // ------- 6. подключение клиента ------
        // isset правшивает у результата select(), был ли server_fd помечен как готовый для чтения
        if (FD_ISSET(server_fd, &readfds)) {
            // если да - принимаем новое соединение
            int new_fd = accept(server_fd, NULL, NULL);
            if (new_fd == -1) {
                perror("accept");
            } else {
                // добавляем в список клиентов
                int placed = 0;
                for (int i = 0; i < FD_SETSIZE; i++) {
                    if (client_fds[i] == -1) {
                        client_fds[i] = new_fd;
                        placed = 1;
                        printf("[SERVER] Новый клиент fd=%d\n", new_fd);
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
            int fd = client_fds[i];
            if (fd != -1 && FD_ISSET(fd, &readfds)) {
                char buf[BUF_SIZE];
                ssize_t n = read(fd, buf, BUF_SIZE);

                if (n > 0) {
                    // переводим в верхний регистр
                    for (ssize_t j = 0; j < n; j++)
                        buf[j] = toupper((unsigned char)buf[j]);

                    // выводим в stdout сервера
                    write(STDOUT_FILENO, buf, n);
                } else if (n == 0) {
                    // клиент отключился
                    printf("[SERVER] Клиент fd=%d отключился\n", fd);
                    close(fd);
                    client_fds[i] = -1;
                } else {
                    perror("read");
                    close(fd);
                    client_fds[i] = -1;
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}






