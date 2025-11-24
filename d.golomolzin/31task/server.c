#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>

#define NUM_CLIENTS 10

// низкая функция чтения из дескриптора клиента (для сервера)
int read_ (int fd) {

    // printf("Результат выполнения работы сервера:\n[upper_text]: ");
    // считываем пока можем по одному символу
    char c;
    int bytes;
    while ( (bytes = read(fd, &c, 1)) > 0) {
        putchar( toupper(c) );
    }
    
    int fl = 0;
    // если ошибка чтения
    if (bytes == -1) {
        perror("read");
        close(fd);
        fl = 1;
    }
    // если просто закрытие
    else if (bytes == 0) {
        close(fd);
        fl = 1;
    }
    return fl;
};


int main () {

    printf("\n┌────────────────────────────────────────┐\n");
    printf("│                 СЕРВЕР                 │\n");
    printf("└────────────────────────────────────────┘\n");

    // создаем сокет для локального взаимодействия
    int server_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }
    
    // хранится же локально, значит можно указать свой путь
    // const char* path_socket = "/mnt/d/Desktop/VSCODE/24941/d.golomolzin/30task/socket";
    const char* path_socket = "/tmp/socket.sock";
    const char* path_socket = "/home/students/24200/d.golomolzin/24941/d.golomolzin/30task/socket";

    // если неполучилось создать И если не ошибка отсутсвия файла (ПРОСТО ОТЧИСТКА ПРЕДЫДУЩЕГО СОКЕТА)
    if (unlink(path_socket) == -1 && errno != 2) {
        perror("unlink");
        close(server_fd);
        return 1;
    }

    // структура для адреса Unix socket
    struct sockaddr_un address;
    // указывает тип связи
    address.sun_family = PF_LOCAL;
    // запишем в address.sun_path строку с путем к сокету
    strncpy(address.sun_path, path_socket, sizeof(address.sun_path) - 1);

    // привяжем к сокету fd локальный адрес struct address длиной sizeof(address)
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    // делаем возможность прослушивания соединения
    if (listen(server_fd, NUM_CLIENTS + 1) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    
    int clients[NUM_CLIENTS];
    for (size_t i = 0; i != NUM_CLIENTS; i++) clients[i] = -1;
    fd_set rfds;
    
    // запустим бесконечный цикл ожидания изменений
    while(1) {
        // очистим набор дескрипторов
        FD_ZERO(&rfds);
        // добавим наш (серверный) дескриптор как главный
        FD_SET(server_fd, &rfds);
        
        // сделаем первой инициализацией максимальный дескриптор серверным
        int mx_df = server_fd;

        // пройдемся по всем дескрипторам
        for (size_t i = 0; i != NUM_CLIENTS; i++) {
            // и добавим их для отслеживания изменений (если не пустой)
            if (clients[i] != -1) {
                FD_SET( clients[i], &rfds );
            }
            
            // select требует первым аргументом номер максимально дескриптора
            mx_df = (clients[i] > mx_df) ? clients[i] : mx_df;
        }

        // модифицирует rfds, оставляя только те дескрипторы, на которых произошли изменения
        int action = select(mx_df + 1, &rfds, NULL, NULL, NULL);
        if (action == -1) {
            perror("select");
            return 1;
        }
        

        /* теперь мы можем проверить каждый дескриптор, чтобы актуализироваться с изменениями */

        // первым делом проверим серверный (вдруг кто-то хочет подключиться)
        if (FD_ISSET(server_fd, &rfds)) {
            
            // принимаем соединение
            // можно рассматривать просто как извлечение из очереди следующего запроса на соединение
            socklen_t len_address = sizeof(address);
            int client_fd = accept(server_fd, (struct sockaddr*)&address, &len_address);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }

            // если удалось принять клиента, то добавим его в отслеживаемые
            int fl = 0; // флаг подключения (1 - получилось / 0 - нет)
            // пройдемся по всему массиву дескрипторов
            for (size_t i = 0; i != NUM_CLIENTS; i++) {
                // если нашли пустую ячейку
                if (clients[i] == -1) {
                    // то запишем в нее дескриптор
                    clients[i] = client_fd;
                    fl = 1;
                    break;
                }
            }
            // если нет места для отслеживания, то просто закроем его
            if (!fl) close(client_fd);
        }

        // теперь можно обработать оставшихся клиентов (которые просто что-то отпраивли текстом)
        for (size_t i = 0; i != NUM_CLIENTS; i++) {
            // если он оказался в этом массиве после отсеивания, то нужно прочитать с него данные
            if (FD_ISSET(clients[i], &rfds)) {
                // считаем данные переданные клентом и выведем их
                if ( read_(clients[i]) ) {
                    // по сути, если для считывания ничего нету, то клиент отключается
                    clients[i] = -1;
                }
            }
        }
    }

    /* тут как-бы в другом файле будет происходить подключение к сокету и отправка текста (клиент) */

    // закроем все оставшиеся дексрипторы
    for (size_t i = 0; i != NUM_CLIENTS; i++) {
        if (clients[i] != -1) close(clients[i]);
    }
    // + серверный конечно же
    close(server_fd);

    return 0;
}