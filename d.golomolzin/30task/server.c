#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>


// низкая функция чтения из дескриптора клиента (для сервера)
void read_ (int fd) {

    printf("Результат выполнения работы сервера:\n[upper_text]: ");
    // считываем пока можем по одному символу
    char c;
    int bytes;
    while ( (bytes = read(fd, &c, 1)) > 0) {
        putchar( toupper(c) );
    }
    printf("\n");
};


int main () {

    printf("┌────────────────────────────────────────┐\n");
    printf("│                 СЕРВЕР                 │\n");
    printf("└────────────────────────────────────────┘\n");

    // создаем сокет для локального взаимодействия
    int fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return 1;
    }
    
    // хранится же локально, значит можно указать свой путь
    // const char* path_socket = "/mnt/d/Desktop/VSCODE/24941/d.golomolzin/30task/socket";
    const char* path_socket = "/tmp/socket";
    // если неполучилось создать И если не ошибка отсутсвия файла (ПРОСТО ОТЧИСТКА ПРЕДЫДУЩЕГО СОКЕТА)
    if (unlink(path_socket) == -1 && errno != 2) {
        perror("unlink");
        close(fd);
        return 1;
    }

    // структура для адреса Unix socket
    struct sockaddr_un address;
    // указывает тип связи
    address.sun_family = PF_LOCAL;
    // запишем в address.sun_path строку с путем к сокету
    strncpy(address.sun_path, path_socket, sizeof(address.sun_path) - 1);

    // привяжем к сокету fd локальный адрес struct address длиной sizeof(address)
    if (bind(fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("bind");
        close(fd);
        return 1;
    }

    // делаем возможность прослушивания соединения (пусть в очереди максимум может быть 1 запрос)
    if (listen(fd, 1) == -1) {
        perror("listen");
        close(fd);
        return 1;
    }

    // принимаем соединения на сокете
    // можно рассматривать просто как извлечение из очереди следующего запроса на соединение
    socklen_t len_address = sizeof(address);
    int client_fd = accept(fd, (struct sockaddr*)&address, &len_address);
    if (client_fd == -1) {
        perror("accept");
        close(fd);
        return 1;
    }

    /* тут как-бы в другом файле будет происходить подключение к сокету и отправка текста (клиент) */

    // получается, в client_fd хранится файловый указатель, по которому client что-то отправил. Обработаем это и результат изменения регистра выведем в терминал
    read_(client_fd);

    close(client_fd);
    close(fd);
    return 0;
}