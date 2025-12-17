#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>

#define MAX_CLIENTS 10
#define SOCKET_PATH "socket31"
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int active;
    int num;
    char buffer[BUFFER_SIZE];
    ssize_t buf_pos;
    struct timespec connect_time;  // Время подключения клиента
} client_t;

client_t clients[MAX_CLIENTS];
int server_fd;

void print_time_event(int num, const char *event) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    struct tm tm_info;
    localtime_r(&ts.tv_sec, &tm_info);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    printf("[%d] %s %s.%03ld\n", num, event, buf, ts.tv_nsec / 1000000);
    fflush(stdout);
}

// Функция для вывода времени, проведенного клиентом на сервере
void print_connection_duration(client_t *c) {
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    
    // Вычисляем разницу во времени
    long seconds = current_time.tv_sec - c->connect_time.tv_sec;
    long nanoseconds = current_time.tv_nsec - c->connect_time.tv_nsec;
    
    // Корректируем, если наносекунды отрицательные
    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1000000000L;
    }
    
    // Конвертируем наносекунды в миллисекунды
    long milliseconds = nanoseconds / 1000000;
    
    printf("[Client %d] Was on server for: %ld.%03ld seconds\n", 
           c->num, seconds, milliseconds);
    fflush(stdout);
}

void handle_client_data(client_t *c) {
    char upper_buf[BUFFER_SIZE];
    int j = 0;
    
    // Преобразуем в верхний регистр
    for (int i = 0; i < c->buf_pos; i++) {
        if (c->buffer[i] == '\n' || c->buffer[i] == '\0') {
            // Завершаем строку и выводим
            if (j > 0) {
                upper_buf[j] = '\0';
                printf("[Client %d] %s\n", c->num, upper_buf);
                fflush(stdout);
                j = 0;
            }
            
            // Если это перевод строки, обрабатываем его отдельно
            if (c->buffer[i] == '\n') {
                // Можно добавить специальную обработку для пустых строк
                printf("[Client %d] (empty line)\n", c->num);
            }
        } else {
            upper_buf[j++] = toupper((unsigned char)c->buffer[i]);
        }
    }
    
    // Если остались данные без завершающего символа
    if (j > 0) {
        upper_buf[j] = '\0';
        printf("[Client %d] %s\n", c->num, upper_buf);
        fflush(stdout);
    }
}

int main() {
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket"); 
        return 1;
    }

    unlink(SOCKET_PATH);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen"); 
        return 1;
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        clients[i].buf_pos = 0;
    }

    printf("Server started. Waiting for connections...\n");
    
    while (1) {
        struct pollfd fds[MAX_CLIENTS + 1];
        int nfds = 0;
        
        // Добавляем серверный сокет
        fds[nfds].fd = server_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        nfds++;
        
        // Добавляем клиентские сокеты
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                fds[nfds].fd = clients[i].fd;
                fds[nfds].events = POLLIN;
                fds[nfds].revents = 0;
                nfds++;
            }
        }
        
        // Ожидаем события
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll");
            continue;
        }
        
        // Проверяем серверный сокет на новые подключения
        if (fds[0].revents & POLLIN) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd >= 0) {
                int idx = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (!clients[i].active) {
                        idx = i;
                        break;
                    }
                }
                
                if (idx >= 0) {
                    clients[idx].fd = fd;
                    clients[idx].active = 1;
                    clients[idx].num = idx + 1;
                    clients[idx].buf_pos = 0;
                    memset(clients[idx].buffer, 0, BUFFER_SIZE);
                    
                    // Запоминаем время подключения клиента
                    clock_gettime(CLOCK_REALTIME, &clients[idx].connect_time);
                    
                    // Делаем клиентский сокет неблокирующим
                    fcntl(fd, F_SETFL, O_NONBLOCK);
                    
                    print_time_event(clients[idx].num, "START");
                    printf("[Client %d] Connected\n", clients[idx].num);
                } else {
                    fprintf(stderr, "No free slots for new client\n");
                    close(fd);
                }
            }
        }
        
        // Обрабатываем данные от клиентов
        int client_index = 1; // Индекс в массиве fds (0 - сервер)
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                if (fds[client_index].revents & POLLIN) {
                    ssize_t n = read(clients[i].fd, 
                                     clients[i].buffer + clients[i].buf_pos, 
                                     BUFFER_SIZE - clients[i].buf_pos - 1);
                    
                    if (n > 0) {
                        clients[i].buf_pos += n;
                        clients[i].buffer[clients[i].buf_pos] = '\0';
                        
                        // Обрабатываем все данные
                        handle_client_data(&clients[i]);
                        clients[i].buf_pos = 0;
                    } else if (n == 0) {
                        // Соединение закрыто клиентом
                        // Выводим время, проведенное на сервере
                        print_connection_duration(&clients[i]);
                        print_time_event(clients[i].num, "END");
                        printf("[Client %d] Disconnected\n", clients[i].num);
                        
                        close(clients[i].fd);
                        clients[i].active = 0;
                    } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("read");
                        close(clients[i].fd);
                        clients[i].active = 0;
                    }
                }
                client_index++;
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}