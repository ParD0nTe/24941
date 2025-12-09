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
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n; // сколько байт прочитали

    // 1. создаем сокет
    // AF_UNIX  → сокет для локального общения через файл
// SOCK_STREAM → потоковое соединение, как TCP
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
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. начинаем слушать
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("сервер запущен, ждет подключения");

    //5. принимаем одно соединение
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("клиент подключился.\n");

    // 6.читаем данные от клиента
    while ((n = read(client_fd, buf, BUF_SIZE)) > 0) {
        // переводим в верхний регистр
        for (ssize_t i = 0; i < n; i++) {
            buf[i] = (char)toupper((unsigned char)buf[i]);
        }
        if (write(STDOUT_FILENO, buf, n) == -1) {
            perror("write");
            break;
        }
    }

    if (n == -1) {
        perror("read");
    } else {
        printf("\nклиент отключился, сервер завершает работу.\n");
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}




