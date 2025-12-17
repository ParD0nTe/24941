#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // read, write, close
#include <sys/socket.h>  // socket, connect
#include <sys/un.h>      // sockaddr_un
#include <time.h>        // rand

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024

int main(void) {
    int sock_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    ssize_t n; // сколько байт прочитали

    // 1. создаем сокет
    // AF_UNIX  → сокет для локального общения через файл (не через интернет)
    // SOCK_STREAM → потоковое соединение, как TCP (надёжная передача байтов)
    // 0 → использовать протокол по умолчанию для такого типа сокета
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 2. заполяем структуру addr( там структура, тип сокета+путь к файлу сокета)
    memset(&addr, 0, sizeof(addr)); // обнуляем весь addr
    addr.sun_family = AF_UNIX;
    // Копируем путь к сокету
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 3. подключаемся к серверу
    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // --- ДОБАВЛЕНО: отправка нескольких сообщений для задачи 32 ---
    srand(time(NULL) ^ getpid());

    const char *msgs[] = {
            "CCC3 hello ",
            "CCC3 part ",
            "CCC3 segment "
    };
    int count_msgs = sizeof(msgs) / sizeof(msgs[0]);

    for (int i = 0; i < 5; i++) {
        const char *m = msgs[rand() % count_msgs];
        write(sock_fd, m, strlen(m));

        usleep((rand() % 300 + 100) * 1000); // задержка 100–400 мс
    }
    // --- КОНЕЦ ДОБАВЛЕНОГО БЛОКА ---

    // 4. читаем из stdin и отправляем всё серверу
    // Можно просто ввести текст руками и нажать Ctrl+D, чтобы закончить
    while ((n = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sock_fd, buf, n) == -1) {
            perror("write");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (n == -1) {
        perror("read");
    }

    // 5. Закрываем сокет — сервер увидит конец потока и завершится
    close(sock_fd);

    return 0;
}

// как запустить
//  gcc client_3.c -o client_3
//  ./client_3
