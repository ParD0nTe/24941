#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


// низкая функция записи в дескриптор
void write_ (int fd) {
    
    char* text = "hello, world and my friend!\n";
    printf("\nЗаписываемый клиентом текст:\n[origin_text]: %s", text);
    printf("\n");
    write(fd, text, strlen(text));
};


int main () {
    
    printf("┌────────────────────────────────────────┐\n");
    printf("│                 КЛИЕНТ                 │\n");
    printf("└────────────────────────────────────────┘");

    // создаем сокет для локального взаимодействия
    int fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return 1;
    }
    
    const char* path_socket = "/home/students/24200/d.golomolzin/24941/d.golomolzin/30task/socket";

    // структура для адреса Unix socket
    struct sockaddr_un address;
    // указывает тип связи
    address.sun_family = PF_LOCAL;
    // запишем в address.sun_path строку с путем к сокету
    strncpy(address.sun_path, path_socket, sizeof(address.sun_path) - 1);

    // пробуем проинициировать подключение к сокету
    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("connect");
        close(fd);
        return 1;
    }

    /* на данном этапе сервер уже слушает и принимает запросы из очереди */

    // запишем в дескриптор текст нижнего регистра
    write_ (fd);

    close(fd);
    return 0;
}